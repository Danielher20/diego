#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

#define MAX_DOCENTES 30
#define MAX_MATERIAS 40
#define MAX_ALUMNOS 400
#define MAX_EVALS 3

#define USER_ADMIN "admin"
#define PASS_ADMIN "admin123"
#define ARCHIVO_DATOS "siga_sencillo.dat"

#define ANCHO_PANTALLA 120
#define ALTO_PANTALLA 30
#define ANCHO_CUADRO_PRINCIPAL 118
#define ALTO_CUADRO_PRINCIPAL 29

typedef struct {
    int id;
    char cedula[20];
    char nombres[50];
    char apellidos[50];
    char usuario[30];
    char clave[30];
    int activo;
} Docente;

typedef struct {
    int id;
    char codigo[20];
    char nombre[60];
    char periodo[20];
    int docenteId;
    char evalNombres[MAX_EVALS][30];
    float ponderaciones[MAX_EVALS];
    int actaGenerada;
} Materia;

typedef struct {
    int id;
    char cedula[20];
    char nombres[50];
    char apellidos[50];
    int materiaId;
    int retirado;
    float notas[MAX_EVALS];
    int tieneNotas[MAX_EVALS];
} Alumno;

Docente docentes[MAX_DOCENTES];
Materia materias[MAX_MATERIAS];
Alumno alumnos[MAX_ALUMNOS];
int totalDocentes = 0;
int totalMaterias = 0;
int totalAlumnos = 0;
int docenteActual = -1;

void configurarConsola(void);
void establecerFondo(void);
void color(int color_val);
void gotoxy(int x, int y);
void limpiarPantalla(void);
void ocultarCursor(void);
void activarCursor(void);
void limpiarArea(int x, int y, int ancho, int alto);
void cuadroPrincipal(void);
void dibujarCuadro(int x, int y, int ancho, int alto);
void centrarTexto(const char *texto, int y);
void mostrarMensajeEnCuadro(const char *mensaje, int x, int y, int ancho, int colorFondo);
void mostrarError(const char *mensaje, int y);
void mostrarExito(const char *mensaje, int y);
void pausa(void);
int esperarTecla(void);
void mostrarBarraCarga(const char *mensaje);
void obtenerFecha(char fecha[20]);
void leerTexto(char *texto, int tamano, int x, int y);
void leerClave(char *texto, int tamano, int x, int y);
int leerEntero(int x, int y);
float leerFloat(int x, int y);
void capitalizar(char *texto);
int menuFlechas(const char *opciones[], int total, int x, int y, int ancho);
void pantallaBase(const char *titulo);

void cargarDatos(void);
void guardarDatos(void);
void colocarPlanDefecto(Materia *materia);
int siguienteIdDocente(void);
int siguienteIdMateria(void);
int siguienteIdAlumno(void);
int validarCedula(const char *cedula);
int validarNombre(const char *nombre);
int buscarDocentePorId(int id);
int buscarDocentePorUsuario(const char *usuario);
int buscarMateriaPorId(int id);
int buscarAlumnoPorCedulaMateria(const char *cedula, int materiaId);
int contarMateriasDocente(int docenteId);
int contarAlumnosMateria(int materiaId);
float calcularDefinitivaAlumno(const Alumno *alumno, const Materia *materia, int *completo);
const char *condicionAlumno(const Alumno *alumno, const Materia *materia);
void nombreDocente(int docenteId, char salida[120]);

int loginAdmin(void);
int loginDocente(void);
void menuPrincipal(void);
void menuAdministrador(void);
void menuDocente(void);

void adminRegistrarDocente(void);
void adminListarDocentes(void);
void adminRegistrarMateria(void);
void adminListarMaterias(void);
void adminAsignarDocente(void);
void adminSupervisarActas(void);
void adminReportesGlobales(void);

int elegirMateriaDelDocente(void);
void docenteRegistrarAlumno(void);
void docenteListarAlumnos(void);
void docentePlanEvaluacion(void);
void docenteCargarNotas(void);
void docenteGenerarActa(void);
void verActaMateria(int materiaId, int modoAdmin);

void configurarConsola(void) {
    char comando[60];
    snprintf(comando, sizeof(comando), "mode con: cols=%d lines=%d", ANCHO_PANTALLA, ALTO_PANTALLA);
    system(comando);
    SetConsoleTitle("SiGA Sencillo - Terminal");
    ocultarCursor();
}

void establecerFondo(void) {
    system("color 97");
}

void color(int color_val) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color_val);
}

void gotoxy(int x, int y) {
    COORD coord;
    coord.X = (SHORT) x;
    coord.Y = (SHORT) y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void limpiarPantalla(void) {
    system("cls");
}

void ocultarCursor(void) {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

void activarCursor(void) {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = TRUE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

void limpiarArea(int x, int y, int ancho, int alto) {
    int i, j;
    color(159);
    for (i = 0; i < alto; i++) {
        gotoxy(x, y + i);
        for (j = 0; j < ancho; j++) {
            printf(" ");
        }
    }
}

void cuadroPrincipal(void) {
    int x = 1;
    int y = 0;
    int ancho = ANCHO_CUADRO_PRINCIPAL;
    int alto = ALTO_CUADRO_PRINCIPAL;
    int fila, col;

    for (fila = 0; fila < alto; fila++) {
        gotoxy(x, y + fila);
        for (col = 0; col < ancho; col++) {
            color(159);
            if (fila == 0 && col == 0) printf("%c", 201);
            else if (fila == 0 && col == ancho - 1) printf("%c", 187);
            else if (fila == alto - 1 && col == 0) printf("%c", 200);
            else if (fila == alto - 1 && col == ancho - 1) printf("%c", 188);
            else if (col == 0 || col == ancho - 1) printf("%c", 186);
            else if (fila == 0 || fila == alto - 1) printf("%c", 205);
            else printf(" ");
        }
    }
}

void dibujarCuadro(int x, int y, int ancho, int alto) {
    int fila, col;

    for (fila = 0; fila < alto; fila++) {
        gotoxy(x, y + fila);
        for (col = 0; col < ancho; col++) {
            color(159);
            if (fila == 0 && col == 0) printf("%c", 218);
            else if (fila == 0 && col == ancho - 1) printf("%c", 191);
            else if (fila == alto - 1 && col == 0) printf("%c", 192);
            else if (fila == alto - 1 && col == ancho - 1) printf("%c", 217);
            else if (col == 0 || col == ancho - 1) printf("%c", 179);
            else if (fila == 0 || fila == alto - 1) printf("%c", 196);
            else printf(" ");
        }
    }
}

void centrarTexto(const char *texto, int y) {
    int longitud = (int) strlen(texto);
    int x = (ANCHO_PANTALLA - longitud) / 2;
    if (x < 0) x = 0;
    gotoxy(x, y);
    printf("%s", texto);
}

void mostrarMensajeEnCuadro(const char *mensaje, int x, int y, int ancho, int colorFondo) {
    int longitud = (int) strlen(mensaje);
    int xCentro = x + (ancho - longitud) / 2;
    if (xCentro < x + 1) xCentro = x + 1;
    gotoxy(xCentro, y);
    color(colorFondo);
    printf("%s", mensaje);
    color(159);
}

void mostrarError(const char *mensaje, int y) {
    color(207);
    centrarTexto(mensaje, y);
    color(159);
}

void mostrarExito(const char *mensaje, int y) {
    color(158);
    centrarTexto(mensaje, y);
    color(159);
}

void pausa(void) {
    color(159);
    centrarTexto("Presione cualquier tecla para continuar...", 27);
    getch();
}

int esperarTecla(void) {
    int tecla = getch();
    if (tecla == 0 || tecla == 224) {
        tecla = getch();
        if (tecla == 72) return 1;
        if (tecla == 80) return 2;
        return tecla;
    }
    if (tecla == 13) return 3;
    if (tecla == 27) return 4;
    return tecla;
}

void mostrarBarraCarga(const char *mensaje) {
    int i;
    pantallaBase("Cargando");
    gotoxy(42, 12);
    printf("%s", mensaje);
    dibujarCuadro(35, 14, 50, 3);
    gotoxy(37, 15);
    for (i = 0; i < 46; i++) {
        color(158);
        printf("%c", 219);
        Sleep(8);
    }
    color(159);
}

void obtenerFecha(char fecha[20]) {
    time_t ahora = time(NULL);
    struct tm *tiempo = localtime(&ahora);
    strftime(fecha, 20, "%d/%m/%Y", tiempo);
}

void leerTexto(char *texto, int tamano, int x, int y) {
    activarCursor();
    color(159);
    gotoxy(x, y);
    fgets(texto, tamano, stdin);
    texto[strcspn(texto, "\n")] = '\0';
    ocultarCursor();
}

void leerClave(char *texto, int tamano, int x, int y) {
    int i = 0;
    int tecla;

    activarCursor();
    gotoxy(x, y);
    while (i < tamano - 1) {
        tecla = getch();
        if (tecla == 13) break;
        if (tecla == 8) {
            if (i > 0) {
                i--;
                texto[i] = '\0';
                printf("\b \b");
            }
        } else if (tecla >= 32 && tecla <= 126) {
            texto[i++] = (char) tecla;
            printf("*");
        }
    }
    texto[i] = '\0';
    ocultarCursor();
}

int leerEntero(int x, int y) {
    char buffer[30];
    leerTexto(buffer, sizeof(buffer), x, y);
    return atoi(buffer);
}

float leerFloat(int x, int y) {
    char buffer[30];
    leerTexto(buffer, sizeof(buffer), x, y);
    return (float) atof(buffer);
}

void capitalizar(char *texto) {
    int i;
    int nuevaPalabra = 1;

    for (i = 0; texto[i] != '\0'; i++) {
        if (isspace((unsigned char) texto[i])) {
            nuevaPalabra = 1;
        } else if (nuevaPalabra) {
            texto[i] = (char) toupper((unsigned char) texto[i]);
            nuevaPalabra = 0;
        } else {
            texto[i] = (char) tolower((unsigned char) texto[i]);
        }
    }
}

int menuFlechas(const char *opciones[], int total, int x, int y, int ancho) {
    int opcion = 0;
    int tecla;
    int i;

    while (1) {
        for (i = 0; i < total; i++) {
            gotoxy(x, y + i * 2);
            if (i == opcion) {
                color(31);
                printf(" > %-*s ", ancho - 4, opciones[i]);
            } else {
                color(159);
                printf("   %-*s ", ancho - 4, opciones[i]);
            }
        }

        tecla = esperarTecla();
        if (tecla == 1) {
            opcion--;
            if (opcion < 0) opcion = total - 1;
        } else if (tecla == 2) {
            opcion++;
            if (opcion >= total) opcion = 0;
        } else if (tecla == 3) {
            return opcion;
        } else if (tecla == 4) {
            return -1;
        }
    }
}

void pantallaBase(const char *titulo) {
    char fecha[20];
    establecerFondo();
    limpiarPantalla();
    cuadroPrincipal();
    dibujarCuadro(11, 2, 99, 3);
    mostrarMensajeEnCuadro(titulo, 11, 3, 99, 159);
    obtenerFecha(fecha);
    gotoxy(88, 6);
    printf("Fecha: %s", fecha);
    gotoxy(8, 6);
    printf("Docentes: %d  Materias: %d  Alumnos: %d", totalDocentes, totalMaterias, totalAlumnos);
}

void colocarPlanDefecto(Materia *materia) {
    strcpy(materia->evalNombres[0], "Parcial 1");
    strcpy(materia->evalNombres[1], "Taller");
    strcpy(materia->evalNombres[2], "Proyecto");
    materia->ponderaciones[0] = 40.0f;
    materia->ponderaciones[1] = 30.0f;
    materia->ponderaciones[2] = 30.0f;
}

void cargarDatos(void) {
    FILE *archivo;
    char magic[8];

    archivo = fopen(ARCHIVO_DATOS, "rb");
    if (archivo == NULL) return;

    if (fread(magic, sizeof(magic), 1, archivo) != 1) {
        fclose(archivo);
        return;
    }

    if (strncmp(magic, "SIGA2", 5) != 0) {
        fclose(archivo);
        return;
    }

    fread(&totalDocentes, sizeof(int), 1, archivo);
    fread(&totalMaterias, sizeof(int), 1, archivo);
    fread(&totalAlumnos, sizeof(int), 1, archivo);

    if (totalDocentes < 0 || totalDocentes > MAX_DOCENTES ||
        totalMaterias < 0 || totalMaterias > MAX_MATERIAS ||
        totalAlumnos < 0 || totalAlumnos > MAX_ALUMNOS) {
        totalDocentes = 0;
        totalMaterias = 0;
        totalAlumnos = 0;
        fclose(archivo);
        return;
    }

    fread(docentes, sizeof(Docente), totalDocentes, archivo);
    fread(materias, sizeof(Materia), totalMaterias, archivo);
    fread(alumnos, sizeof(Alumno), totalAlumnos, archivo);
    fclose(archivo);
}

void guardarDatos(void) {
    FILE *archivo;
    char magic[8] = "SIGA2";

    archivo = fopen(ARCHIVO_DATOS, "wb");
    if (archivo == NULL) {
        mostrarError("No se pudo guardar el archivo de datos.", 25);
        return;
    }

    fwrite(magic, sizeof(magic), 1, archivo);
    fwrite(&totalDocentes, sizeof(int), 1, archivo);
    fwrite(&totalMaterias, sizeof(int), 1, archivo);
    fwrite(&totalAlumnos, sizeof(int), 1, archivo);
    fwrite(docentes, sizeof(Docente), totalDocentes, archivo);
    fwrite(materias, sizeof(Materia), totalMaterias, archivo);
    fwrite(alumnos, sizeof(Alumno), totalAlumnos, archivo);
    fclose(archivo);
}

int siguienteIdDocente(void) {
    int i, mayor = 0;
    for (i = 0; i < totalDocentes; i++) {
        if (docentes[i].id > mayor) mayor = docentes[i].id;
    }
    return mayor + 1;
}

int siguienteIdMateria(void) {
    int i, mayor = 0;
    for (i = 0; i < totalMaterias; i++) {
        if (materias[i].id > mayor) mayor = materias[i].id;
    }
    return mayor + 1;
}

int siguienteIdAlumno(void) {
    int i, mayor = 0;
    for (i = 0; i < totalAlumnos; i++) {
        if (alumnos[i].id > mayor) mayor = alumnos[i].id;
    }
    return mayor + 1;
}

int validarCedula(const char *cedula) {
    int i;
    if (strlen(cedula) < 6) return 0;
    for (i = 0; cedula[i] != '\0'; i++) {
        if (!isdigit((unsigned char) cedula[i])) return 0;
    }
    return 1;
}

int validarNombre(const char *nombre) {
    int i;
    int letras = 0;
    for (i = 0; nombre[i] != '\0'; i++) {
        if (isalpha((unsigned char) nombre[i])) letras++;
        else if (!isspace((unsigned char) nombre[i]) && nombre[i] != '.') return 0;
    }
    return letras >= 2;
}

int buscarDocentePorId(int id) {
    int i;
    for (i = 0; i < totalDocentes; i++) {
        if (docentes[i].id == id) return i;
    }
    return -1;
}

int buscarDocentePorUsuario(const char *usuario) {
    int i;
    for (i = 0; i < totalDocentes; i++) {
        if (strcmp(docentes[i].usuario, usuario) == 0) return i;
    }
    return -1;
}

int buscarMateriaPorId(int id) {
    int i;
    for (i = 0; i < totalMaterias; i++) {
        if (materias[i].id == id) return i;
    }
    return -1;
}

int buscarAlumnoPorCedulaMateria(const char *cedula, int materiaId) {
    int i;
    for (i = 0; i < totalAlumnos; i++) {
        if (alumnos[i].materiaId == materiaId && strcmp(alumnos[i].cedula, cedula) == 0) return i;
    }
    return -1;
}

int contarMateriasDocente(int docenteId) {
    int i, total = 0;
    for (i = 0; i < totalMaterias; i++) {
        if (materias[i].docenteId == docenteId) total++;
    }
    return total;
}

int contarAlumnosMateria(int materiaId) {
    int i, total = 0;
    for (i = 0; i < totalAlumnos; i++) {
        if (alumnos[i].materiaId == materiaId && !alumnos[i].retirado) total++;
    }
    return total;
}

float calcularDefinitivaAlumno(const Alumno *alumno, const Materia *materia, int *completo) {
    int i;
    float definitiva = 0.0f;
    *completo = 1;

    if (alumno->retirado) {
        *completo = 1;
        return 0.0f;
    }

    for (i = 0; i < MAX_EVALS; i++) {
        if (!alumno->tieneNotas[i]) {
            *completo = 0;
            return 0.0f;
        }
        definitiva += alumno->notas[i] * (materia->ponderaciones[i] / 100.0f);
    }

    return definitiva;
}

const char *condicionAlumno(const Alumno *alumno, const Materia *materia) {
    int completo;
    float definitiva;

    if (alumno->retirado) return "RETIRADO";
    definitiva = calcularDefinitivaAlumno(alumno, materia, &completo);
    if (!completo) return "INCOMPLETO";
    if (definitiva >= 10.0f) return "APROBADO";
    return "REPROBADO";
}

void nombreDocente(int docenteId, char salida[120]) {
    int pos = buscarDocentePorId(docenteId);
    if (pos < 0) {
        strcpy(salida, "Sin asignar");
        return;
    }
    snprintf(salida, 120, "%s %s", docentes[pos].nombres, docentes[pos].apellidos);
}

int loginAdmin(void) {
    char usuario[30], clave[30];

    pantallaBase("LOGIN ADMINISTRADOR");
    dibujarCuadro(35, 9, 50, 10);
    gotoxy(42, 12);
    printf("Usuario: ");
    leerTexto(usuario, sizeof(usuario), 51, 12);
    gotoxy(42, 14);
    printf("Clave: ");
    leerClave(clave, sizeof(clave), 49, 14);

    if (strcmp(usuario, USER_ADMIN) == 0 && strcmp(clave, PASS_ADMIN) == 0) {
        mostrarBarraCarga("Entrando al panel del director...");
        return 1;
    }

    mostrarError("Usuario o clave de administrador incorrecta.", 22);
    pausa();
    return 0;
}

int loginDocente(void) {
    char usuario[30], clave[30];
    int pos;

    pantallaBase("LOGIN DOCENTE");
    dibujarCuadro(35, 9, 50, 10);
    gotoxy(42, 12);
    printf("Usuario: ");
    leerTexto(usuario, sizeof(usuario), 51, 12);
    gotoxy(42, 14);
    printf("Clave: ");
    leerClave(clave, sizeof(clave), 49, 14);

    pos = buscarDocentePorUsuario(usuario);
    if (pos >= 0 && docentes[pos].activo && strcmp(docentes[pos].clave, clave) == 0) {
        docenteActual = pos;
        mostrarBarraCarga("Entrando al panel del docente...");
        return 1;
    }

    mostrarError("Usuario o clave de docente incorrecta.", 22);
    pausa();
    return 0;
}

void menuPrincipal(void) {
    const char *opciones[] = {
        "Panel Administrador",
        "Panel Docente",
        "Salir"
    };
    int opcion;

    while (1) {
        pantallaBase("SISTEMA DE GESTION ACADEMICA SENCILLO");
        dibujarCuadro(37, 10, 46, 10);
        mostrarMensajeEnCuadro("Seleccione con flechas y ENTER", 37, 11, 46, 159);
        opcion = menuFlechas(opciones, 3, 42, 14, 36);

        if (opcion == 0) {
            if (loginAdmin()) menuAdministrador();
        } else if (opcion == 1) {
            if (loginDocente()) menuDocente();
        } else {
            guardarDatos();
            break;
        }
    }
}

void menuAdministrador(void) {
    const char *opciones[] = {
        "Registrar docente y crear usuario",
        "Listar docentes",
        "Registrar materia",
        "Listar materias",
        "Asignar docente a materia",
        "Supervision de actas",
        "Reportes globales",
        "Guardar datos",
        "Cerrar sesion"
    };
    int opcion;

    while (1) {
        pantallaBase("PANEL ADMINISTRADOR - DIRECTOR");
        dibujarCuadro(30, 8, 60, 18);
        opcion = menuFlechas(opciones, 9, 35, 10, 50);

        if (opcion == 0) adminRegistrarDocente();
        else if (opcion == 1) adminListarDocentes();
        else if (opcion == 2) adminRegistrarMateria();
        else if (opcion == 3) adminListarMaterias();
        else if (opcion == 4) adminAsignarDocente();
        else if (opcion == 5) adminSupervisarActas();
        else if (opcion == 6) adminReportesGlobales();
        else if (opcion == 7) {
            guardarDatos();
            mostrarExito("Datos guardados correctamente.", 25);
            pausa();
        } else {
            guardarDatos();
            break;
        }
    }
}

void menuDocente(void) {
    const char *opciones[] = {
        "Registrar alumno en materia",
        "Listado de alumnos",
        "Plan de evaluacion",
        "Carga de notas",
        "Generar / ver acta",
        "Guardar datos",
        "Cerrar sesion"
    };
    char titulo[140];
    int opcion;

    while (1) {
        snprintf(titulo, sizeof(titulo), "PANEL DOCENTE - %s %s", docentes[docenteActual].nombres, docentes[docenteActual].apellidos);
        pantallaBase(titulo);
        gotoxy(8, 7);
        printf("Materias asignadas: %d", contarMateriasDocente(docentes[docenteActual].id));
        dibujarCuadro(31, 9, 58, 16);
        opcion = menuFlechas(opciones, 7, 36, 11, 48);

        if (opcion == 0) docenteRegistrarAlumno();
        else if (opcion == 1) docenteListarAlumnos();
        else if (opcion == 2) docentePlanEvaluacion();
        else if (opcion == 3) docenteCargarNotas();
        else if (opcion == 4) docenteGenerarActa();
        else if (opcion == 5) {
            guardarDatos();
            mostrarExito("Datos guardados correctamente.", 25);
            pausa();
        } else {
            guardarDatos();
            docenteActual = -1;
            break;
        }
    }
}

void adminRegistrarDocente(void) {
    Docente nuevo;
    char buffer[130];

    pantallaBase("REGISTRAR DOCENTE");
    if (totalDocentes >= MAX_DOCENTES) {
        mostrarError("No hay espacio para mas docentes.", 23);
        pausa();
        return;
    }

    dibujarCuadro(18, 8, 84, 16);
    gotoxy(24, 10); printf("Cedula: ");
    leerTexto(nuevo.cedula, sizeof(nuevo.cedula), 45, 10);
    if (!validarCedula(nuevo.cedula)) {
        mostrarError("Cedula invalida. Use solo numeros y minimo 6 digitos.", 25);
        pausa();
        return;
    }

    gotoxy(24, 12); printf("Nombres: ");
    leerTexto(nuevo.nombres, sizeof(nuevo.nombres), 45, 12);
    if (!validarNombre(nuevo.nombres)) {
        mostrarError("Nombre invalido.", 25);
        pausa();
        return;
    }
    capitalizar(nuevo.nombres);

    gotoxy(24, 14); printf("Apellidos: ");
    leerTexto(nuevo.apellidos, sizeof(nuevo.apellidos), 45, 14);
    if (!validarNombre(nuevo.apellidos)) {
        mostrarError("Apellido invalido.", 25);
        pausa();
        return;
    }
    capitalizar(nuevo.apellidos);

    gotoxy(24, 16); printf("Usuario para el docente: ");
    leerTexto(nuevo.usuario, sizeof(nuevo.usuario), 45, 16);
    if (strlen(nuevo.usuario) < 3 || buscarDocentePorUsuario(nuevo.usuario) >= 0) {
        mostrarError("Usuario invalido o ya existe.", 25);
        pausa();
        return;
    }

    gotoxy(24, 18); printf("Clave asignada por admin: ");
    leerTexto(nuevo.clave, sizeof(nuevo.clave), 45, 18);
    if (strlen(nuevo.clave) < 4) {
        mostrarError("La clave debe tener minimo 4 caracteres.", 25);
        pausa();
        return;
    }

    nuevo.id = siguienteIdDocente();
    nuevo.activo = 1;
    docentes[totalDocentes++] = nuevo;
    guardarDatos();

    snprintf(buffer, sizeof(buffer), "Docente creado. Usuario: %s  Clave: %s", nuevo.usuario, nuevo.clave);
    mostrarExito(buffer, 25);
    pausa();
}

void adminListarDocentes(void) {
    int i;
    int y = 10;

    pantallaBase("LISTADO DE DOCENTES");
    dibujarCuadro(7, 8, 106, 17);
    gotoxy(10, 9);
    printf("ID   CEDULA       DOCENTE                         USUARIO          MATERIAS  ESTADO");

    for (i = 0; i < totalDocentes && i < 13; i++) {
        gotoxy(10, y + i);
        printf("%-4d %-12s %-30s %-15s %-8d %s",
               docentes[i].id,
               docentes[i].cedula,
               docentes[i].apellidos,
               docentes[i].usuario,
               contarMateriasDocente(docentes[i].id),
               docentes[i].activo ? "Activo" : "Suspendido");
    }

    if (totalDocentes == 0) {
        mostrarError("No hay docentes registrados.", 17);
    } else if (totalDocentes > 13) {
        gotoxy(10, 24);
        printf("Mostrando primeros 13 docentes de %d.", totalDocentes);
    }
    pausa();
}

void adminRegistrarMateria(void) {
    Materia nueva;
    int docenteId;
    int posDocente;

    pantallaBase("REGISTRAR MATERIA");
    if (totalMaterias >= MAX_MATERIAS) {
        mostrarError("No hay espacio para mas materias.", 23);
        pausa();
        return;
    }

    dibujarCuadro(18, 8, 84, 15);
    gotoxy(24, 10); printf("Codigo: ");
    leerTexto(nueva.codigo, sizeof(nueva.codigo), 45, 10);
    if (strlen(nueva.codigo) < 2) {
        mostrarError("Codigo invalido.", 24);
        pausa();
        return;
    }

    gotoxy(24, 12); printf("Nombre de materia: ");
    leerTexto(nueva.nombre, sizeof(nueva.nombre), 45, 12);
    if (strlen(nueva.nombre) < 3) {
        mostrarError("Nombre de materia invalido.", 24);
        pausa();
        return;
    }
    capitalizar(nueva.nombre);

    gotoxy(24, 14); printf("Periodo: ");
    leerTexto(nueva.periodo, sizeof(nueva.periodo), 45, 14);
    if (strlen(nueva.periodo) == 0) strcpy(nueva.periodo, "2026-1");

    gotoxy(24, 16); printf("ID docente (0 sin asignar): ");
    docenteId = leerEntero(52, 16);
    if (docenteId != 0) {
        posDocente = buscarDocentePorId(docenteId);
        if (posDocente < 0 || !docentes[posDocente].activo) {
            mostrarError("Docente no existe o esta suspendido.", 24);
            pausa();
            return;
        }
    }

    nueva.id = siguienteIdMateria();
    nueva.docenteId = docenteId;
    nueva.actaGenerada = 0;
    colocarPlanDefecto(&nueva);
    materias[totalMaterias++] = nueva;
    guardarDatos();

    mostrarExito("Materia registrada correctamente.", 24);
    pausa();
}

void adminListarMaterias(void) {
    int i;
    int y = 10;
    char docente[120];

    pantallaBase("LISTADO DE MATERIAS");
    dibujarCuadro(5, 8, 110, 17);
    gotoxy(8, 9);
    printf("ID   CODIGO      MATERIA                       PERIODO     DOCENTE              ALUMNOS ACTA");

    for (i = 0; i < totalMaterias && i < 13; i++) {
        nombreDocente(materias[i].docenteId, docente);
        gotoxy(8, y + i);
        printf("%-4d %-11s %-28s %-10s %-20s %-7d %s",
               materias[i].id,
               materias[i].codigo,
               materias[i].nombre,
               materias[i].periodo,
               docente,
               contarAlumnosMateria(materias[i].id),
               materias[i].actaGenerada ? "Si" : "No");
    }

    if (totalMaterias == 0) {
        mostrarError("No hay materias registradas.", 17);
    } else if (totalMaterias > 13) {
        gotoxy(8, 24);
        printf("Mostrando primeras 13 materias de %d.", totalMaterias);
    }
    pausa();
}

void adminAsignarDocente(void) {
    int idMateria, idDocente;
    int posMateria, posDocente;

    pantallaBase("ASIGNAR DOCENTE A MATERIA");
    dibujarCuadro(22, 9, 76, 11);
    gotoxy(28, 12); printf("ID materia: ");
    idMateria = leerEntero(48, 12);
    posMateria = buscarMateriaPorId(idMateria);
    if (posMateria < 0) {
        mostrarError("Materia no encontrada.", 23);
        pausa();
        return;
    }

    gotoxy(28, 14); printf("ID docente: ");
    idDocente = leerEntero(48, 14);
    posDocente = buscarDocentePorId(idDocente);
    if (posDocente < 0 || !docentes[posDocente].activo) {
        mostrarError("Docente no encontrado o suspendido.", 23);
        pausa();
        return;
    }

    materias[posMateria].docenteId = idDocente;
    guardarDatos();
    mostrarExito("Docente asignado a la materia.", 23);
    pausa();
}

void adminSupervisarActas(void) {
    int idMateria;
    int posMateria;

    pantallaBase("SUPERVISION DE ACTAS");
    adminListarMaterias();

    pantallaBase("SUPERVISION DE ACTAS");
    dibujarCuadro(25, 10, 70, 8);
    gotoxy(31, 13);
    printf("Ingrese ID de la materia para ver su acta: ");
    idMateria = leerEntero(72, 13);
    posMateria = buscarMateriaPorId(idMateria);
    if (posMateria < 0) {
        mostrarError("Materia no encontrada.", 23);
        pausa();
        return;
    }
    verActaMateria(idMateria, 1);
}

void adminReportesGlobales(void) {
    int i;
    int aprobados = 0, reprobados = 0, incompletos = 0, retirados = 0;
    int completo;
    float definitiva;
    int posMateria;

    for (i = 0; i < totalAlumnos; i++) {
        posMateria = buscarMateriaPorId(alumnos[i].materiaId);
        if (posMateria < 0) continue;
        if (alumnos[i].retirado) {
            retirados++;
            continue;
        }
        definitiva = calcularDefinitivaAlumno(&alumnos[i], &materias[posMateria], &completo);
        if (!completo) incompletos++;
        else if (definitiva >= 10.0f) aprobados++;
        else reprobados++;
    }

    pantallaBase("REPORTES GLOBALES");
    dibujarCuadro(25, 8, 70, 16);
    gotoxy(34, 11); printf("Docentes registrados:       %d", totalDocentes);
    gotoxy(34, 12); printf("Materias registradas:       %d", totalMaterias);
    gotoxy(34, 13); printf("Alumnos registrados:        %d", totalAlumnos);
    gotoxy(34, 15); printf("Aprobados:                  %d", aprobados);
    gotoxy(34, 16); printf("Reprobados:                 %d", reprobados);
    gotoxy(34, 17); printf("Incompletos:                %d", incompletos);
    gotoxy(34, 18); printf("Retirados:                  %d", retirados);
    gotoxy(34, 20); printf("Actas generadas:            ");

    {
        int actas = 0;
        for (i = 0; i < totalMaterias; i++) {
            if (materias[i].actaGenerada) actas++;
        }
        printf("%d", actas);
    }

    pausa();
}

int elegirMateriaDelDocente(void) {
    int i;
    int idMateria;
    int posMateria;
    int y = 11;
    int docenteId = docentes[docenteActual].id;

    pantallaBase("MATERIAS DEL DOCENTE");
    dibujarCuadro(10, 8, 100, 16);
    gotoxy(14, 9);
    printf("ID   CODIGO      MATERIA                       PERIODO     ALUMNOS  ACTA");

    for (i = 0; i < totalMaterias; i++) {
        if (materias[i].docenteId == docenteId) {
            gotoxy(14, y++);
            printf("%-4d %-11s %-28s %-10s %-8d %s",
                   materias[i].id,
                   materias[i].codigo,
                   materias[i].nombre,
                   materias[i].periodo,
                   contarAlumnosMateria(materias[i].id),
                   materias[i].actaGenerada ? "Si" : "No");
        }
    }

    if (y == 11) {
        mostrarError("Este docente no tiene materias asignadas por el admin.", 22);
        pausa();
        return -1;
    }

    gotoxy(14, 24);
    printf("ID de la materia: ");
    idMateria = leerEntero(32, 24);
    posMateria = buscarMateriaPorId(idMateria);

    if (posMateria < 0 || materias[posMateria].docenteId != docenteId) {
        mostrarError("Materia no valida para este docente.", 26);
        pausa();
        return -1;
    }

    return idMateria;
}

void docenteRegistrarAlumno(void) {
    Alumno nuevo;
    int materiaId;
    int i;

    materiaId = elegirMateriaDelDocente();
    if (materiaId < 0) return;

    pantallaBase("REGISTRAR ALUMNO");
    if (totalAlumnos >= MAX_ALUMNOS) {
        mostrarError("No hay espacio para mas alumnos.", 23);
        pausa();
        return;
    }

    dibujarCuadro(20, 9, 80, 13);
    gotoxy(26, 11); printf("Cedula: ");
    leerTexto(nuevo.cedula, sizeof(nuevo.cedula), 48, 11);
    if (!validarCedula(nuevo.cedula)) {
        mostrarError("Cedula invalida.", 24);
        pausa();
        return;
    }
    if (buscarAlumnoPorCedulaMateria(nuevo.cedula, materiaId) >= 0) {
        mostrarError("Ese alumno ya esta registrado en esta materia.", 24);
        pausa();
        return;
    }

    gotoxy(26, 13); printf("Nombres: ");
    leerTexto(nuevo.nombres, sizeof(nuevo.nombres), 48, 13);
    if (!validarNombre(nuevo.nombres)) {
        mostrarError("Nombre invalido.", 24);
        pausa();
        return;
    }
    capitalizar(nuevo.nombres);

    gotoxy(26, 15); printf("Apellidos: ");
    leerTexto(nuevo.apellidos, sizeof(nuevo.apellidos), 48, 15);
    if (!validarNombre(nuevo.apellidos)) {
        mostrarError("Apellido invalido.", 24);
        pausa();
        return;
    }
    capitalizar(nuevo.apellidos);

    nuevo.id = siguienteIdAlumno();
    nuevo.materiaId = materiaId;
    nuevo.retirado = 0;
    for (i = 0; i < MAX_EVALS; i++) {
        nuevo.notas[i] = 0.0f;
        nuevo.tieneNotas[i] = 0;
    }

    alumnos[totalAlumnos++] = nuevo;
    guardarDatos();
    mostrarExito("Alumno registrado y asignado a la materia.", 24);
    pausa();
}

void docenteListarAlumnos(void) {
    int materiaId;
    int posMateria;
    int i;
    int y = 11;
    int completo;
    float definitiva;

    materiaId = elegirMateriaDelDocente();
    if (materiaId < 0) return;
    posMateria = buscarMateriaPorId(materiaId);

    pantallaBase("LISTADO DE ALUMNOS");
    dibujarCuadro(4, 8, 112, 17);
    gotoxy(8, 9);
    printf("Materia: %s", materias[posMateria].nombre);
    gotoxy(8, 10);
    printf("CEDULA       ALUMNO                         N1     N2     N3     DEFINITIVA  CONDICION");

    for (i = 0; i < totalAlumnos && y < 24; i++) {
        if (alumnos[i].materiaId == materiaId) {
            definitiva = calcularDefinitivaAlumno(&alumnos[i], &materias[posMateria], &completo);
            gotoxy(8, y++);
            printf("%-12s %-30s %5.2f  %5.2f  %5.2f  %8s  %s",
                   alumnos[i].cedula,
                   alumnos[i].apellidos,
                   alumnos[i].tieneNotas[0] ? alumnos[i].notas[0] : 0.0f,
                   alumnos[i].tieneNotas[1] ? alumnos[i].notas[1] : 0.0f,
                   alumnos[i].tieneNotas[2] ? alumnos[i].notas[2] : 0.0f,
                   completo ? "" : "Pend.",
                   condicionAlumno(&alumnos[i], &materias[posMateria]));
            if (completo) {
                gotoxy(82, y - 1);
                printf("%8.2f", definitiva);
            }
        }
    }

    if (y == 11) {
        mostrarError("No hay alumnos registrados en esta materia.", 18);
    }
    pausa();
}

void docentePlanEvaluacion(void) {
    int materiaId;
    int posMateria;
    int i, j;
    int hayNotas = 0;
    float suma = 0.0f;
    float diferencia;
    char nombreEval[30];
    float pesoEval[MAX_EVALS];
    char nombresEval[MAX_EVALS][30];

    materiaId = elegirMateriaDelDocente();
    if (materiaId < 0) return;
    posMateria = buscarMateriaPorId(materiaId);

    for (i = 0; i < totalAlumnos; i++) {
        if (alumnos[i].materiaId == materiaId) {
            for (j = 0; j < MAX_EVALS; j++) {
                if (alumnos[i].tieneNotas[j]) hayNotas = 1;
            }
        }
    }

    pantallaBase("PLAN DE EVALUACION");
    dibujarCuadro(15, 8, 90, 17);
    gotoxy(22, 10);
    printf("Materia: %s", materias[posMateria].nombre);

    if (hayNotas) {
        gotoxy(22, 12);
        printf("El plan no se puede editar porque ya existen notas cargadas.");
        gotoxy(22, 14);
        printf("Plan actual:");
        for (i = 0; i < MAX_EVALS; i++) {
            gotoxy(24, 16 + i);
            printf("%d. %-20s %.2f%%", i + 1, materias[posMateria].evalNombres[i], materias[posMateria].ponderaciones[i]);
        }
        pausa();
        return;
    }

    gotoxy(22, 12);
    printf("Configure 3 evaluaciones. La suma debe ser 100%%.");
    for (i = 0; i < MAX_EVALS; i++) {
        gotoxy(22, 15 + i * 2);
        printf("Nombre eval %d: ", i + 1);
        leerTexto(nombreEval, sizeof(nombreEval), 40, 15 + i * 2);
        if (strlen(nombreEval) == 0) {
            snprintf(nombreEval, sizeof(nombreEval), "Evaluacion %d", i + 1);
        }
        gotoxy(70, 15 + i * 2);
        printf("Peso: ");
        pesoEval[i] = leerFloat(76, 15 + i * 2);
        if (pesoEval[i] <= 0.0f) {
            mostrarError("Cada ponderacion debe ser mayor que cero.", 25);
            pausa();
            return;
        }
        strcpy(nombresEval[i], nombreEval);
        suma += pesoEval[i];
    }

    diferencia = suma - 100.0f;
    if (diferencia < 0.0f) diferencia = -diferencia;
    if (diferencia > 0.1f) {
        mostrarError("La suma de ponderaciones debe ser 100%.", 25);
        pausa();
        return;
    }

    for (i = 0; i < MAX_EVALS; i++) {
        strcpy(materias[posMateria].evalNombres[i], nombresEval[i]);
        materias[posMateria].ponderaciones[i] = pesoEval[i];
    }

    guardarDatos();
    mostrarExito("Plan de evaluacion guardado correctamente.", 25);
    pausa();
}

void docenteCargarNotas(void) {
    int materiaId;
    int posMateria;
    int posAlumno;
    int i;
    char cedula[20];
    float nota;

    materiaId = elegirMateriaDelDocente();
    if (materiaId < 0) return;
    posMateria = buscarMateriaPorId(materiaId);

    pantallaBase("CARGA DE NOTAS");
    dibujarCuadro(15, 8, 90, 17);
    gotoxy(22, 10);
    printf("Materia: %s", materias[posMateria].nombre);
    gotoxy(22, 12);
    printf("Cedula del alumno: ");
    leerTexto(cedula, sizeof(cedula), 43, 12);

    posAlumno = buscarAlumnoPorCedulaMateria(cedula, materiaId);
    if (posAlumno < 0) {
        mostrarError("Alumno no encontrado en esta materia.", 25);
        pausa();
        return;
    }
    if (alumnos[posAlumno].retirado) {
        mostrarError("El alumno esta retirado.", 25);
        pausa();
        return;
    }

    gotoxy(22, 14);
    printf("Alumno: %s %s", alumnos[posAlumno].nombres, alumnos[posAlumno].apellidos);
    for (i = 0; i < MAX_EVALS; i++) {
        gotoxy(22, 16 + i * 2);
        printf("%s (%.0f%%) nota 0-20: ", materias[posMateria].evalNombres[i], materias[posMateria].ponderaciones[i]);
        nota = leerFloat(55, 16 + i * 2);
        if (nota < 0.0f || nota > 20.0f) {
            mostrarError("La nota debe estar entre 0 y 20.", 25);
            pausa();
            return;
        }
        alumnos[posAlumno].notas[i] = nota;
        alumnos[posAlumno].tieneNotas[i] = 1;
    }

    materias[posMateria].actaGenerada = 0;
    guardarDatos();
    mostrarExito("Notas guardadas. Si eran existentes, quedaron corregidas.", 25);
    pausa();
}

void docenteGenerarActa(void) {
    int materiaId;
    int posMateria;

    materiaId = elegirMateriaDelDocente();
    if (materiaId < 0) return;
    posMateria = buscarMateriaPorId(materiaId);
    materias[posMateria].actaGenerada = 1;
    guardarDatos();
    verActaMateria(materiaId, 0);
}

void verActaMateria(int materiaId, int modoAdmin) {
    int posMateria = buscarMateriaPorId(materiaId);
    int i;
    int y = 13;
    int completo;
    float definitiva;
    char docente[120];

    if (posMateria < 0) {
        mostrarError("Materia no encontrada.", 23);
        pausa();
        return;
    }

    nombreDocente(materias[posMateria].docenteId, docente);
    pantallaBase(modoAdmin ? "ACTA SUPERVISADA POR ADMIN" : "ACTA FINAL DE MATERIA");
    dibujarCuadro(4, 8, 112, 18);
    gotoxy(8, 9);
    printf("Materia: %s  Codigo: %s  Periodo: %s", materias[posMateria].nombre, materias[posMateria].codigo, materias[posMateria].periodo);
    gotoxy(8, 10);
    printf("Docente: %s", docente);
    gotoxy(8, 12);
    printf("CEDULA       ALUMNO                         %-8s %-8s %-8s DEF.     CONDICION",
           materias[posMateria].evalNombres[0],
           materias[posMateria].evalNombres[1],
           materias[posMateria].evalNombres[2]);

    for (i = 0; i < totalAlumnos && y < 25; i++) {
        if (alumnos[i].materiaId == materiaId) {
            definitiva = calcularDefinitivaAlumno(&alumnos[i], &materias[posMateria], &completo);
            gotoxy(8, y++);
            printf("%-12s %-30s %7.2f  %7.2f  %7.2f  %6s   %s",
                   alumnos[i].cedula,
                   alumnos[i].apellidos,
                   alumnos[i].tieneNotas[0] ? alumnos[i].notas[0] : 0.0f,
                   alumnos[i].tieneNotas[1] ? alumnos[i].notas[1] : 0.0f,
                   alumnos[i].tieneNotas[2] ? alumnos[i].notas[2] : 0.0f,
                   completo ? "" : "Pend.",
                   condicionAlumno(&alumnos[i], &materias[posMateria]));
            if (completo) {
                gotoxy(83, y - 1);
                printf("%6.2f", definitiva);
            }
        }
    }

    if (y == 13) {
        mostrarError("No hay alumnos para esta acta.", 18);
    }

    gotoxy(8, 26);
    printf("Acta generada: %s", materias[posMateria].actaGenerada ? "Si" : "No");
    pausa();
}

int main(void) {
    configurarConsola();
    cargarDatos();
    mostrarBarraCarga("Iniciando SiGA sencillo...");
    menuPrincipal();
    activarCursor();
    color(7);
    limpiarPantalla();
    return 0;
}
