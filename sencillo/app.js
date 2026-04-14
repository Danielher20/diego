var alumnos = [];
var usuarioActual = "";
var rolActual = "";

var ponderaciones = [40, 30, 30];

function mostrarMensaje(texto) {
  var toast = document.getElementById("toast");
  toast.innerText = texto;
  toast.classList.add("ver");

  setTimeout(function () {
    toast.classList.remove("ver");
  }, 2500);
}

function iniciarSesion(evento) {
  evento.preventDefault();

  var usuario = document.getElementById("usuario").value;
  var clave = document.getElementById("clave").value;
  var mensaje = document.getElementById("mensaje-login");

  if (usuario === "admin" && clave === "1234") {
    usuarioActual = usuario;
    rolActual = "ADMIN";
  } else if (usuario === "docente" && clave === "1234") {
    usuarioActual = usuario;
    rolActual = "DOCENTE";
  } else {
    mensaje.innerText = "Usuario o clave incorrectos.";
    return;
  }

  mensaje.innerText = "";
  document.getElementById("login").classList.add("oculto");
  document.getElementById("app").classList.remove("oculto");
  document.getElementById("rol-actual").innerText = rolActual;
  document.getElementById("btn-admin").style.display = rolActual === "ADMIN" ? "block" : "none";

  cargarDatos();
  mostrarPagina("inicio");
  actualizarTodo();
}

function cerrarSesion() {
  usuarioActual = "";
  rolActual = "";
  document.getElementById("usuario").value = "";
  document.getElementById("clave").value = "";
  document.getElementById("app").classList.add("oculto");
  document.getElementById("login").classList.remove("oculto");
}

function mostrarPagina(nombre) {
  var paginas = document.querySelectorAll(".pagina");
  var i;

  for (i = 0; i < paginas.length; i++) {
    paginas[i].classList.remove("visible");
  }

  document.getElementById("pagina-" + nombre).classList.add("visible");

  var titulos = {
    inicio: "Inicio",
    alumnos: "Alumnos",
    notas: "Notas",
    acta: "Acta final",
    admin: "Administrador"
  };

  document.getElementById("titulo-pagina").innerText = titulos[nombre];
  actualizarTodo();
}

function guardarDatos() {
  var datos = {
    alumnos: alumnos,
    ponderaciones: ponderaciones
  };

  localStorage.setItem("siga_sencillo_datos", JSON.stringify(datos));
  mostrarMensaje("Datos guardados.");
}

function cargarDatos() {
  var texto = localStorage.getItem("siga_sencillo_datos");
  var datos;

  if (texto === null) {
    alumnos = [
      {
        cedula: "V-25100001",
        nombres: "Ana",
        apellidos: "Rodriguez",
        retirado: false,
        notas: [15, 16, 14]
      },
      {
        cedula: "V-25100002",
        nombres: "Carlos",
        apellidos: "Mendez",
        retirado: false,
        notas: [9, 11, 10]
      }
    ];
    return;
  }

  datos = JSON.parse(texto);
  alumnos = datos.alumnos || [];
  ponderaciones = datos.ponderaciones || [40, 30, 30];
}

function registrarAlumno() {
  var cedula = document.getElementById("cedula").value;
  var nombres = document.getElementById("nombres").value;
  var apellidos = document.getElementById("apellidos").value;
  var i;

  if (cedula === "" || nombres === "" || apellidos === "") {
    mostrarMensaje("Completa todos los campos.");
    return;
  }

  for (i = 0; i < alumnos.length; i++) {
    if (alumnos[i].cedula === cedula) {
      mostrarMensaje("Ya existe esa cedula.");
      return;
    }
  }

  alumnos.push({
    cedula: cedula,
    nombres: nombres,
    apellidos: apellidos,
    retirado: false,
    notas: ["", "", ""]
  });

  document.getElementById("cedula").value = "";
  document.getElementById("nombres").value = "";
  document.getElementById("apellidos").value = "";

  actualizarTodo();
  mostrarMensaje("Alumno registrado.");
}

function retirarAlumno(posicion) {
  alumnos[posicion].retirado = true;
  actualizarTodo();
  mostrarMensaje("Alumno retirado.");
}

function cambiarNota(posicion, indiceNota, valor) {
  var nota = Number(valor);

  if (valor === "") {
    alumnos[posicion].notas[indiceNota] = "";
    actualizarTodo();
    return;
  }

  if (nota < 0 || nota > 20) {
    mostrarMensaje("La nota debe estar entre 0 y 20.");
    actualizarTodo();
    return;
  }

  alumnos[posicion].notas[indiceNota] = nota;
  actualizarTodo();
}

function calcularDefinitiva(alumno) {
  var total = 0;
  var i;

  if (alumno.retirado) {
    return null;
  }

  for (i = 0; i < 3; i++) {
    if (alumno.notas[i] === "") {
      return null;
    }

    total = total + Number(alumno.notas[i]) * (ponderaciones[i] / 100);
  }

  return Math.round(total * 100) / 100;
}

function cambiarPonderaciones() {
  var p1 = Number(document.getElementById("pond1").value);
  var p2 = Number(document.getElementById("pond2").value);
  var p3 = Number(document.getElementById("pond3").value);
  var suma = p1 + p2 + p3;

  if (suma !== 100) {
    document.getElementById("mensaje-admin").innerText = "La suma debe ser 100. Ahora suma " + suma + ".";
    return;
  }

  ponderaciones = [p1, p2, p3];
  document.getElementById("mensaje-admin").innerText = "Ponderaciones actualizadas.";
  actualizarTodo();
}

function actualizarTodo() {
  pintarResumen();
  pintarAlumnos();
  pintarNotas();
  pintarActa();
  document.getElementById("pond1").value = ponderaciones[0];
  document.getElementById("pond2").value = ponderaciones[1];
  document.getElementById("pond3").value = ponderaciones[2];
}

function pintarResumen() {
  var aprobados = 0;
  var reprobados = 0;
  var retirados = 0;
  var def;
  var i;

  for (i = 0; i < alumnos.length; i++) {
    def = calcularDefinitiva(alumnos[i]);

    if (alumnos[i].retirado) {
      retirados++;
    } else if (def !== null && def >= 10) {
      aprobados++;
    } else if (def !== null && def < 10) {
      reprobados++;
    }
  }

  document.getElementById("total-alumnos").innerText = alumnos.length;
  document.getElementById("total-aprobados").innerText = aprobados;
  document.getElementById("total-reprobados").innerText = reprobados;
  document.getElementById("total-retirados").innerText = retirados;
}

function pintarAlumnos() {
  var cuerpo = document.getElementById("tabla-alumnos");
  var html = "";
  var i;

  for (i = 0; i < alumnos.length; i++) {
    html += "<tr>";
    html += "<td>" + alumnos[i].cedula + "</td>";
    html += "<td><strong>" + alumnos[i].apellidos + "</strong>, " + alumnos[i].nombres + "</td>";
    html += "<td>" + estadoAlumno(alumnos[i]) + "</td>";
    html += "<td>";
    if (!alumnos[i].retirado) {
      html += "<button onclick='retirarAlumno(" + i + ")'>Retirar</button>";
    }
    html += "</td>";
    html += "</tr>";
  }

  cuerpo.innerHTML = html;
}

function pintarNotas() {
  var cuerpo = document.getElementById("tabla-notas");
  var html = "";
  var i;
  var def;

  for (i = 0; i < alumnos.length; i++) {
    def = calcularDefinitiva(alumnos[i]);

    html += "<tr>";
    html += "<td><strong>" + alumnos[i].apellidos + "</strong>, " + alumnos[i].nombres + "</td>";

    if (alumnos[i].retirado) {
      html += "<td colspan='4'>Alumno retirado</td>";
    } else {
      html += "<td><input type='number' min='0' max='20' value='" + alumnos[i].notas[0] + "' onchange='cambiarNota(" + i + ",0,this.value)'></td>";
      html += "<td><input type='number' min='0' max='20' value='" + alumnos[i].notas[1] + "' onchange='cambiarNota(" + i + ",1,this.value)'></td>";
      html += "<td><input type='number' min='0' max='20' value='" + alumnos[i].notas[2] + "' onchange='cambiarNota(" + i + ",2,this.value)'></td>";
      html += "<td>" + (def === null ? "Incompleto" : def.toFixed(2)) + "</td>";
    }

    html += "</tr>";
  }

  cuerpo.innerHTML = html;
}

function pintarActa() {
  var caja = document.getElementById("acta");
  var html = "";
  var i;
  var def;
  var condicion;

  html += "<h2>Acta final - Programacion I</h2>";
  html += "<p>Periodo 2026-I</p>";
  html += "<div class='tabla'><table>";
  html += "<thead><tr><th>Cedula</th><th>Alumno</th><th>N1</th><th>N2</th><th>N3</th><th>Def.</th><th>Condicion</th></tr></thead>";
  html += "<tbody>";

  for (i = 0; i < alumnos.length; i++) {
    def = calcularDefinitiva(alumnos[i]);

    if (alumnos[i].retirado) {
      condicion = "RETIRADO";
    } else if (def === null) {
      condicion = "INCOMPLETO";
    } else if (def >= 10) {
      condicion = "APROBADO";
    } else {
      condicion = "REPROBADO";
    }

    html += "<tr>";
    html += "<td>" + alumnos[i].cedula + "</td>";
    html += "<td>" + alumnos[i].apellidos + ", " + alumnos[i].nombres + "</td>";
    html += "<td>" + mostrarNota(alumnos[i].notas[0]) + "</td>";
    html += "<td>" + mostrarNota(alumnos[i].notas[1]) + "</td>";
    html += "<td>" + mostrarNota(alumnos[i].notas[2]) + "</td>";
    html += "<td>" + (def === null ? "-" : def.toFixed(2)) + "</td>";
    html += "<td>" + condicion + "</td>";
    html += "</tr>";
  }

  html += "</tbody></table></div>";
  caja.innerHTML = html;
}

function estadoAlumno(alumno) {
  var def = calcularDefinitiva(alumno);

  if (alumno.retirado) {
    return "<span class='estado retirado'>Retirado</span>";
  }

  if (def === null) {
    return "<span class='estado mal'>Incompleto</span>";
  }

  if (def >= 10) {
    return "<span class='estado ok'>Aprobado</span>";
  }

  return "<span class='estado mal'>Reprobado</span>";
}

function mostrarNota(nota) {
  if (nota === "") {
    return "-";
  }

  return Number(nota).toFixed(2);
}

function exportarCSV() {
  var contenido = "Cedula,Apellidos,Nombres,N1,N2,N3,Definitiva\n";
  var i;
  var def;
  var enlace;

  for (i = 0; i < alumnos.length; i++) {
    def = calcularDefinitiva(alumnos[i]);
    contenido += alumnos[i].cedula + ",";
    contenido += alumnos[i].apellidos + ",";
    contenido += alumnos[i].nombres + ",";
    contenido += mostrarNota(alumnos[i].notas[0]) + ",";
    contenido += mostrarNota(alumnos[i].notas[1]) + ",";
    contenido += mostrarNota(alumnos[i].notas[2]) + ",";
    contenido += (def === null ? "" : def.toFixed(2)) + "\n";
  }

  enlace = document.createElement("a");
  enlace.href = "data:text/csv;charset=utf-8," + encodeURIComponent(contenido);
  enlace.download = "acta_sencilla.csv";
  enlace.click();
}

cargarDatos();
