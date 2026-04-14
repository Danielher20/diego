const state = {
  token: localStorage.getItem("siga_token") || "",
  user: null,
  materias: [],
  docentes: [],
  materiaId: 0,
  evaluaciones: [],
  alumnos: [],
  notasSnapshot: new Map(),
};

const $ = (sel) => document.querySelector(sel);
const $$ = (sel) => Array.from(document.querySelectorAll(sel));

function toast(message) {
  const el = $("#toast");
  el.textContent = message;
  el.classList.add("show");
  clearTimeout(el._timer);
  el._timer = setTimeout(() => el.classList.remove("show"), 3200);
}

function encode(data) {
  const params = new URLSearchParams();
  Object.entries(data).forEach(([key, value]) => params.set(key, value ?? ""));
  return params.toString();
}

async function api(path, options = {}) {
  const method = options.method || "GET";
  const data = { ...(options.data || {}) };
  if (state.token) data.token = state.token;
  const init = { method, headers: {} };
  let url = path;
  if (method === "GET") {
    const qs = encode(data);
    if (qs) url += (url.includes("?") ? "&" : "?") + qs;
  } else {
    init.headers["Content-Type"] = "application/x-www-form-urlencoded";
    init.body = encode(data);
  }
  const res = await fetch(url, init);
  const text = await res.text();
  const payload = (res.headers.get("content-type") || "").includes("application/json")
    ? JSON.parse(text || "{}")
    : text;
  if (!res.ok || (payload && payload.ok === false)) {
    throw new Error(payload.error || `Error HTTP ${res.status}`);
  }
  return payload;
}

function showLogin() {
  $("#login-view").classList.remove("hidden");
  $("#app-view").classList.add("hidden");
}

function showApp() {
  $("#login-view").classList.add("hidden");
  $("#app-view").classList.remove("hidden");
  $("#session-name").textContent = state.user.nombre;
  $("#session-role").textContent = state.user.rol;
  $("#admin-nav").classList.toggle("hidden", state.user.rol !== "ADMIN");
  $("#docente-nav").classList.toggle("hidden", state.user.rol !== "DOCENTE");
  showPage(state.user.rol === "ADMIN" ? "admin-dashboard" : "doc-dashboard");
}

function showPage(page) {
  $$(".page").forEach((el) => el.classList.remove("active"));
  $(`#page-${page}`)?.classList.add("active");
  $$(".nav button").forEach((el) => el.classList.toggle("active", el.dataset.page === page));
  const labels = {
    "admin-dashboard": "Dashboard",
    "admin-docentes": "Docentes",
    "admin-materias": "Materias",
    "admin-actas": "Actas",
    "admin-logs": "Auditoría",
    "doc-dashboard": "Mis materias",
    "doc-alumnos": "Alumnos",
    "doc-plan": "Plan de evaluación",
    "doc-notas": "Cargar notas",
    "doc-acta": "Acta final",
  };
  $("#page-title").textContent = labels[page] || "SiGA";
  refreshCurrent();
}

async function refreshCurrent() {
  if (!state.user) return;
  try {
    if (state.user.rol === "ADMIN") {
      await Promise.all([loadAdminStats(), loadDocentes()]);
      await loadMaterias();
      if ($("#page-admin-actas").classList.contains("active")) await loadAdminActa();
      if ($("#page-admin-logs").classList.contains("active")) await loadLogs();
    } else {
      await loadDocenteMaterias();
      if (state.materiaId) {
        if ($("#page-doc-alumnos").classList.contains("active")) await loadAlumnos();
        if ($("#page-doc-plan").classList.contains("active")) await loadPlan();
        if ($("#page-doc-notas").classList.contains("active")) await loadNotas();
        if ($("#page-doc-acta").classList.contains("active")) await loadDocActa();
      }
    }
  } catch (err) {
    toast(err.message);
  }
}

async function login(event) {
  event.preventDefault();
  $("#login-error").textContent = "";
  try {
    const payload = await api("/api/login", {
      method: "POST",
      data: { username: $("#login-user").value.trim(), password: $("#login-pass").value },
    });
    state.token = payload.token;
    state.user = payload.usuario;
    localStorage.setItem("siga_token", state.token);
    showApp();
    await refreshCurrent();
  } catch (err) {
    $("#login-error").textContent = err.message;
  }
}

async function restoreSession() {
  if (!state.token) return showLogin();
  try {
    const payload = await api("/api/session");
    state.user = payload.usuario;
    showApp();
    await refreshCurrent();
  } catch {
    localStorage.removeItem("siga_token");
    state.token = "";
    showLogin();
  }
}

async function logout() {
  try { await api("/api/logout", { method: "POST" }); } catch {}
  state.token = "";
  state.user = null;
  localStorage.removeItem("siga_token");
  showLogin();
}

async function loadAdminStats() {
  if (state.user?.rol !== "ADMIN") return;
  const s = await api("/api/admin/stats");
  $("#stat-docentes").textContent = s.docentes_activos;
  $("#stat-materias").textContent = s.materias_activas;
  $("#stat-alumnos").textContent = s.total_alumnos;
  $("#stat-rendimiento").textContent = `${s.aprobados} / ${s.reprobados}`;
}

async function loadDocentes() {
  if (state.user?.rol !== "ADMIN") return;
  const payload = await api("/api/admin/docentes");
  state.docentes = payload.items;
  $("#admin-docentes-body").innerHTML = payload.items.map((d) => `
    <tr>
      <td class="mono">${d.cedula}</td>
      <td><strong>${d.apellidos}</strong>, ${d.nombres}<br><small>${d.email}</small></td>
      <td class="mono">${d.username}</td>
      <td>${d.materias.length ? d.materias.join(", ") : "<span class='pill warn'>Sin materia</span>"}</td>
      <td><span class="pill ${d.activo ? "ok" : "bad"}">${d.activo ? "Activo" : "Suspendido"}</span></td>
      <td>
        <button class="btn ghost" onclick="resetDocente(${d.id})">Reset</button>
        ${d.activo ? `<button class="btn danger" onclick="suspenderDocente(${d.id})">Suspender</button>` : ""}
      </td>
    </tr>
  `).join("");
  const options = `<option value="0">Sin asignar</option>` + payload.items
    .filter((d) => d.activo)
    .map((d) => `<option value="${d.id}">${d.apellidos}, ${d.nombres}</option>`)
    .join("");
  $("#materia-docente-select").innerHTML = options;
}

async function loadMaterias() {
  if (state.user?.rol !== "ADMIN") return;
  const payload = await api("/api/admin/materias");
  state.materias = payload.items;
  $("#admin-materias-body").innerHTML = payload.items.map((m) => `
    <tr>
      <td class="mono">${m.codigo}</td>
      <td><strong>${m.nombre}</strong></td>
      <td>${m.periodo}</td>
      <td>${m.docente}</td>
      <td><span class="pill ${m.acta_generada ? "ok" : "warn"}">${m.acta_generada ? "Generada" : "Pendiente"}</span></td>
      <td>
        <select onchange="asignarMateria(${m.id}, this.value)">
          <option value="0">Sin asignar</option>
          ${state.docentes.filter((d) => d.activo).map((d) => `<option value="${d.id}" ${d.id === m.docente_id ? "selected" : ""}>${d.apellidos}, ${d.nombres}</option>`).join("")}
        </select>
      </td>
    </tr>
  `).join("");
  $("#admin-acta-materia").innerHTML = payload.items
    .map((m) => `<option value="${m.id}">${m.codigo} - ${m.nombre}</option>`)
    .join("");
}

async function createDocente(event) {
  event.preventDefault();
  const form = new FormData(event.target);
  const payload = await api("/api/admin/docentes", { method: "POST", data: Object.fromEntries(form) });
  event.target.reset();
  $("#docente-dialog").close();
  toast(`Docente creado. Usuario: ${payload.credenciales.username} | Clave: ${payload.credenciales.password_temporal}`);
  await refreshCurrent();
}

async function createMateria(event) {
  event.preventDefault();
  const form = new FormData(event.target);
  await api("/api/admin/materias", { method: "POST", data: Object.fromEntries(form) });
  event.target.reset();
  $("#materia-dialog").close();
  toast("Materia creada correctamente");
  await refreshCurrent();
}

async function resetDocente(id) {
  const payload = await api("/api/admin/docentes/reset", { method: "POST", data: { id } });
  toast(`Contraseña temporal: ${payload.password_temporal}`);
}

async function suspenderDocente(id) {
  if (!confirm("¿Suspender este docente? Sus datos se conservarán.")) return;
  await api("/api/admin/docentes/suspender", { method: "POST", data: { id } });
  toast("Docente suspendido");
  await refreshCurrent();
}

async function asignarMateria(id, docente_id) {
  await api("/api/admin/materias/asignar", { method: "POST", data: { id, docente_id } });
  toast("Materia actualizada");
  await refreshCurrent();
}

async function loadLogs() {
  const payload = await api("/api/admin/logs");
  $("#logs-body").innerHTML = payload.items.map((log) => `
    <tr><td class="mono">${log.timestamp}</td><td>${log.usuario_id}</td><td>${log.accion}</td><td>${log.detalle}</td></tr>
  `).join("");
}

async function loadDocenteMaterias() {
  if (state.user?.rol !== "DOCENTE") return;
  const payload = await api("/api/docente/materias");
  state.materias = payload.items;
  if (!state.materiaId && payload.items.length) state.materiaId = payload.items[0].id;
  $("#doc-materia-select").innerHTML = payload.items
    .map((m) => `<option value="${m.id}" ${m.id === state.materiaId ? "selected" : ""}>${m.codigo} - ${m.nombre}</option>`)
    .join("");
  $("#doc-materia-cards").innerHTML = payload.items.map((m) => `
    <article class="card">
      <h4>${m.codigo} - ${m.nombre}</h4>
      <p>${m.periodo}</p>
      <p><span class="pill ${m.acta_generada ? "ok" : "warn"}">${m.acta_generada ? "Acta generada" : "Acta pendiente"}</span></p>
    </article>
  `).join("");
}

async function loadAlumnos() {
  const payload = await api("/api/docente/alumnos", { data: { materia_id: state.materiaId } });
  state.alumnos = payload.items;
  $("#alumnos-body").innerHTML = payload.items.map((a) => `
    <tr>
      <td class="mono">${a.cedula}</td>
      <td><strong>${a.apellidos}</strong>, ${a.nombres}</td>
      <td><span class="pill ${a.retirado ? "bad" : "ok"}">${a.retirado ? "Retirado" : "Activo"}</span></td>
      <td>${a.retirado ? "" : `<button class="btn danger" onclick="retirarAlumno(${a.id})">Retirar</button>`}</td>
    </tr>
  `).join("");
}

async function createAlumno(event) {
  event.preventDefault();
  const form = new FormData(event.target);
  await api("/api/docente/alumnos", {
    method: "POST",
    data: { materia_id: state.materiaId, ...Object.fromEntries(form) },
  });
  event.target.reset();
  $("#alumno-dialog").close();
  toast("Alumno registrado");
  await loadAlumnos();
}

async function retirarAlumno(id) {
  if (!confirm("¿Marcar este alumno como retirado?")) return;
  await api("/api/docente/alumnos/retirar", { method: "POST", data: { id } });
  toast("Alumno retirado");
  await loadAlumnos();
}

async function loadPlan() {
  const payload = await api("/api/docente/evaluaciones", { data: { materia_id: state.materiaId } });
  state.evaluaciones = payload.items.length
    ? payload.items
    : [{ nombre: "Parcial 1", tipo: "PARCIAL", ponderacion: 100 }];
  renderPlan(payload.bloqueado);
  renderEvalSelect();
}

function renderPlan(bloqueado = false) {
  $("#plan-body").innerHTML = state.evaluaciones.map((ev, index) => `
    <tr>
      <td><input value="${ev.nombre}" ${bloqueado ? "disabled" : ""} oninput="state.evaluaciones[${index}].nombre=this.value;updatePlanTotal()"></td>
      <td>
        <select ${bloqueado ? "disabled" : ""} onchange="state.evaluaciones[${index}].tipo=this.value">
          ${["PARCIAL","TALLER","PRACTICA","TRABAJO","OTRO"].map((t) => `<option ${ev.tipo === t ? "selected" : ""}>${t}</option>`).join("")}
        </select>
      </td>
      <td><input type="number" min="5" max="100" step="0.01" value="${ev.ponderacion}" ${bloqueado ? "disabled" : ""} oninput="state.evaluaciones[${index}].ponderacion=Number(this.value);updatePlanTotal()"></td>
      <td>${bloqueado ? "<span class='pill warn'>Bloqueado</span>" : `<button class="btn danger" onclick="removeEval(${index})">Quitar</button>`}</td>
    </tr>
  `).join("");
  $("#add-eval-btn").disabled = bloqueado;
  $("#save-plan-btn").disabled = bloqueado;
  updatePlanTotal();
}

function updatePlanTotal() {
  const total = state.evaluaciones.reduce((sum, ev) => sum + Number(ev.ponderacion || 0), 0);
  $("#plan-status").textContent = `${total.toFixed(2)}%`;
  $("#plan-status").className = `pill ${total === 100 ? "ok" : total > 100 ? "bad" : "warn"}`;
  $("#plan-bar").style.width = `${Math.min(total, 100)}%`;
  $("#plan-bar").className = total === 100 ? "complete" : total > 100 ? "over" : "";
}

function addEval() {
  state.evaluaciones.push({ nombre: "Nueva evaluación", tipo: "OTRO", ponderacion: 5 });
  renderPlan(false);
}

function removeEval(index) {
  state.evaluaciones.splice(index, 1);
  renderPlan(false);
}

async function savePlan() {
  const items = state.evaluaciones
    .map((ev) => `${String(ev.nombre).replace(/[;|]/g, " ")}|${ev.tipo}|${Number(ev.ponderacion || 0).toFixed(2)}`)
    .join(";");
  await api("/api/docente/evaluaciones", { method: "POST", data: { materia_id: state.materiaId, items } });
  toast("Plan guardado");
  await loadPlan();
}

function renderEvalSelect() {
  $("#eval-select").innerHTML = state.evaluaciones
    .filter((ev) => ev.id)
    .map((ev) => `<option value="${ev.id}">${ev.nombre} (${Number(ev.ponderacion).toFixed(2)}%)</option>`)
    .join("");
}

async function loadNotas() {
  if (!state.evaluaciones.length || !state.evaluaciones[0].id) await loadPlan();
  const evalId = Number($("#eval-select").value || state.evaluaciones[0]?.id || 0);
  if (!evalId) {
    $("#notas-body").innerHTML = `<tr><td colspan="4">Configura el plan de evaluación primero.</td></tr>`;
    return;
  }
  const payload = await api("/api/docente/calificaciones", {
    data: { materia_id: state.materiaId, evaluacion_id: evalId },
  });
  state.notasSnapshot.clear();
  $("#notas-body").innerHTML = payload.items.map((row) => {
    state.notasSnapshot.set(row.alumno_id, row.nota);
    return `
      <tr>
        <td class="mono">${row.cedula}</td>
        <td><strong>${row.apellidos}</strong>, ${row.nombres}</td>
        <td><input class="nota-input" data-alumno="${row.alumno_id}" type="number" min="0" max="20" step="0.01" value="${row.nota ?? ""}" placeholder="0.00"></td>
        <td><span class="pill ${row.nota === null ? "warn" : row.nota >= 10 ? "ok" : "bad"}">${row.nota === null ? "Pendiente" : row.nota >= 10 ? "OK" : "Baja"}</span></td>
      </tr>
    `;
  }).join("");
}

async function saveNotas() {
  const evalId = Number($("#eval-select").value);
  const inputs = $$(".nota-input");
  let changedExisting = false;
  const items = inputs.map((inp) => {
    const alumno = Number(inp.dataset.alumno);
    const value = Number(inp.value);
    if (Number.isNaN(value) || value < 0 || value > 20) throw new Error("Todas las notas deben estar entre 0 y 20");
    const prev = state.notasSnapshot.get(alumno);
    if (prev !== null && prev !== undefined && Number(prev).toFixed(2) !== value.toFixed(2)) changedExisting = true;
    return `${alumno}:${value.toFixed(2)}`;
  }).join(";");
  let motivo = "";
  if (changedExisting) {
    motivo = prompt("Hay notas existentes modificadas. Indica el motivo de corrección:");
    if (!motivo) return toast("Motivo obligatorio para corregir notas");
  }
  await api("/api/docente/calificaciones", { method: "POST", data: { evaluacion_id: evalId, items, motivo } });
  toast("Calificaciones guardadas");
  await loadNotas();
}

function renderActa(target, payload) {
  const evalHeaders = payload.evaluaciones
    .map((ev) => `<th>${ev.nombre}<br><small>${ev.ponderacion.toFixed(2)}%</small></th>`)
    .join("");
  const rows = payload.alumnos.map((a) => `
    <tr>
      <td class="mono">${a.cedula}</td>
      <td><strong>${a.apellidos}</strong>, ${a.nombres}</td>
      ${a.notas.map((n) => `<td class="mono">${n === null ? "-" : n.toFixed(2)}</td>`).join("")}
      <td class="mono">${a.definitiva === null ? "-" : a.definitiva.toFixed(2)}</td>
      <td><span class="pill ${a.condicion === "APROBADO" ? "ok" : a.condicion === "REPROBADO" ? "bad" : "warn"}">${a.condicion}</span></td>
    </tr>
  `).join("");
  target.innerHTML = `
    <h3>Acta final - ${payload.materia.codigo} ${payload.materia.nombre}</h3>
    <div class="acta-meta">Período ${payload.materia.periodo} · Aprobados ${payload.resumen.aprobados} · Reprobados ${payload.resumen.reprobados} · Retirados ${payload.resumen.retirados}</div>
    <div class="table-wrap">
      <table>
        <thead><tr><th>Cédula</th><th>Alumno</th>${evalHeaders}<th>Def.</th><th>Condición</th></tr></thead>
        <tbody>${rows}</tbody>
      </table>
    </div>
  `;
}

async function loadDocActa() {
  const payload = await api("/api/docente/acta", { data: { materia_id: state.materiaId } });
  renderActa($("#doc-acta"), payload);
}

async function loadAdminActa() {
  const materiaId = Number($("#admin-acta-materia").value || state.materias[0]?.id || 0);
  if (!materiaId) return;
  const payload = await api("/api/admin/acta", { data: { materia_id: materiaId } });
  renderActa($("#admin-acta"), payload);
}

async function markActa() {
  await api("/api/docente/acta/generar", { method: "POST", data: { materia_id: state.materiaId } });
  toast("Acta marcada como generada");
  await loadDocActa();
}

function openCsv() {
  const path = state.user.rol === "ADMIN" ? "/api/admin/acta.csv" : "/api/docente/acta.csv";
  const materiaId = state.user.rol === "ADMIN" ? Number($("#admin-acta-materia").value) : state.materiaId;
  window.open(`${path}?token=${encodeURIComponent(state.token)}&materia_id=${materiaId}`, "_blank");
}

function wireEvents() {
  $("#login-form").addEventListener("submit", login);
  $("#logout-btn").addEventListener("click", logout);
  $("#refresh-btn").addEventListener("click", refreshCurrent);
  $$(".nav button").forEach((btn) => btn.addEventListener("click", () => showPage(btn.dataset.page)));
  $$("[data-open]").forEach((btn) => btn.addEventListener("click", () => $(`#${btn.dataset.open}`).showModal()));
  $("#docente-form").addEventListener("submit", createDocente);
  $("#materia-form").addEventListener("submit", createMateria);
  $("#alumno-form").addEventListener("submit", createAlumno);
  $("#doc-materia-select").addEventListener("change", async (event) => {
    state.materiaId = Number(event.target.value);
    await refreshCurrent();
  });
  $("#admin-acta-materia").addEventListener("change", loadAdminActa);
  $("#add-eval-btn").addEventListener("click", addEval);
  $("#save-plan-btn").addEventListener("click", () => savePlan().catch((err) => toast(err.message)));
  $("#eval-select").addEventListener("change", () => loadNotas().catch((err) => toast(err.message)));
  $("#save-notas-btn").addEventListener("click", () => saveNotas().catch((err) => toast(err.message)));
  $("#print-btn").addEventListener("click", () => window.print());
  $("#csv-btn").addEventListener("click", openCsv);
  $("#mark-acta-btn").addEventListener("click", () => markActa().catch((err) => toast(err.message)));
}

window.resetDocente = (id) => resetDocente(id).catch((err) => toast(err.message));
window.suspenderDocente = (id) => suspenderDocente(id).catch((err) => toast(err.message));
window.asignarMateria = (id, docenteId) => asignarMateria(id, docenteId).catch((err) => toast(err.message));
window.retirarAlumno = (id) => retirarAlumno(id).catch((err) => toast(err.message));
window.removeEval = removeEval;
window.state = state;
window.updatePlanTotal = updatePlanTotal;

wireEvents();
restoreSession();
