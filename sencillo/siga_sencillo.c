#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <windows.h>

#define MAX_SECCIONES 30
#define MAX_ALUMNOS 300
#define MAX_EVALUACIONES 20
#define NOTA_MINIMA 0.0f
#define NOTA_MAXIMA 20.0f
#define NOTA_APROBATORIA 10.0f

#define COLOR_NORMAL 15
#define COLOR_TITULO 11
#define COLOR_EXITO 10
#define COLOR_ERROR 12

#define OPCION_INVALIDA -1
#define OPCION_CANCELADA -2

typedef struct {
    char nombre[50];
    char apellido[50];
    char usuario[40];
    char clave[40];
    int creado;
} Usuario;

typedef struct {
    char descripcion[60];
    float ponderacion;
} Evaluacion;

typedef struct {
    int id;
    char nombre[60];
    Evaluacion evaluaciones[MAX_EVALUACIONES];
    int totalEvaluaciones;
} Seccion;

typedef struct {
    int seccionId;
    char cedula[20];
    char nombre[50];
    char apellido[50];
    float notas[MAX_EVALUACIONES];
    int tieneNota[MAX_EVALUACIONES];
} Alumno;

Usuario usuarioPrincipal;
Seccion secciones[MAX_SECCIONES];
Alumno alumnos[MAX_ALUMNOS];

int totalSecciones = 0;
int totalAlumnos = 0;
int seccionActiva = -1;
int siguienteSeccionId = 1;

void color(int valor);
void gotoxy(int x, int y);
void limpiarPantalla(void);
void configurarConsola(void);
void cabecera(const char *titulo);
void pausaMenu(void);
void pausaReintentar(void);
void mensajeError(const char *mensaje);
void mensajeExito(const char *mensaje);
int leerLinea(char *destino, int tamano);
int leerEnPosicion(char *destino, int tamano, int x, int y);
int leerClaveEnPosicion(char *destino, int tamano, int x, int y);
int textoVacio(const char *texto);
int soloNumeros(const char *texto);
int textoANumero(const char *texto, int *valor);
int textoAFloat(const char *texto, float *valor);
int confirmar(const char *mensaje);
int confirmarSalida(void);
int confirmarGuardar(void);
int confirmarEdicion(void);
int confirmarEliminacion(void);
int leerOpcion(int x, int y);
int pedirTextoObligatorio(char *destino, int tamano, int x, int y);
int pedirCedula(char *destino, int tamano, int x, int y);
int pedirFloat(float *valor, int x, int y);
void mostrarTablaAlumnos(int seccionId, int yInicio);
void mostrarListaEvaluaciones(int indiceSeccion, int yInicio);
int contarAlumnosSeccion(int seccionId);
int buscarSeccionPorNombre(const char *nombre, int ignorarIndice);
int buscarAlumnoPorCedula(const char *cedula, int seccionId);
int indiceSeccionPorId(int id);
float sumaPonderaciones(int indiceSeccion, int ignorarIndice);
float notaDefinitiva(const Alumno *alumno, const Seccion *seccion);
const char *estadoAlumno(float definitiva);
int seleccionarSeccion(void);
int seleccionarAlumno(void);
int seleccionarEvaluacion(int indiceSeccion);
void eliminarAlumnoPorIndice(int indice);
void eliminarEvaluacion(int indiceSeccion, int indiceEvaluacion);
void eliminarSeccionPorIndice(int indiceSeccion);

void menuPrincipal(void);
int crearUsuario(void);
int menuIngreso(void);
void menuGestionSecciones(void);
void crearSeccion(void);
void modificarSeccion(void);
void borrarSeccion(void);
void menuSistema(void);
void menuAlumnos(void);
void registrarAlumno(void);
void editarAlumno(void);
void retirarAlumno(void);
void menuPlanEvaluacion(void);
void registrarEvaluacion(void);
void editarEvaluacion(void);
void borrarEvaluacion(void);
void menuNotas(void);
void cargarEditarCalificacion(void);
void verResumenAlumno(void);
void menuActas(void);
void visualizarActa(void);

void color(int valor) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), valor);
}

void gotoxy(int x, int y) {
    COORD posicion;
    posicion.X = (SHORT)x;
    posicion.Y = (SHORT)y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), posicion);
}

void limpiarPantalla(void) {
    system("cls");
}

void configurarConsola(void) {
    system("mode con: cols=120 lines=30");
    SetConsoleTitleA("SIGA sencillo");
}

void cabecera(const char *titulo) {
    limpiarPantalla();
    color(COLOR_TITULO);
    gotoxy(4, 1);
    printf("==============================================================");
    gotoxy(4, 2);
    printf("%s", titulo);
    gotoxy(84, 2);
    printf("ESC: regresar");
    gotoxy(4, 3);
    printf("==============================================================");
    color(COLOR_NORMAL);
}

void pausaMenu(void) {
    color(COLOR_NORMAL);
    gotoxy(4, 27);
    printf("Presione cualquier tecla para regresar al menu...");
    getch();
}

void pausaReintentar(void) {
    color(COLOR_NORMAL);
    gotoxy(4, 27);
    printf("Presione cualquier tecla para reintentar...");
    getch();
}

void mensajeError(const char *mensaje) {
    color(COLOR_ERROR);
    gotoxy(4, 25);
    printf("%-110s", mensaje);
    color(COLOR_NORMAL);
}

void mensajeExito(const char *mensaje) {
    color(COLOR_EXITO);
    gotoxy(4, 25);
    printf("%-110s", mensaje);
    color(COLOR_NORMAL);
}

int leerLinea(char *destino, int tamano) {
    int i = 0;
    int tecla;

    destino[0] = '\0';
    while (i < tamano - 1) {
        tecla = getch();
        if (tecla == 27) {
            destino[0] = '\0';
            return 0;
        }
        if (tecla == 13) {
            break;
        }
        if (tecla == 8) {
            if (i > 0) {
                i--;
                destino[i] = '\0';
                printf("\b \b");
            }
            continue;
        }
        if (tecla == 0 || tecla == 224) {
            getch();
            continue;
        }
        if (tecla >= 32 && tecla <= 126) {
            destino[i] = (char)tecla;
            i++;
            destino[i] = '\0';
            putchar(tecla);
        }
    }

    destino[i] = '\0';
    return 1;
}

int leerEnPosicion(char *destino, int tamano, int x, int y) {
    gotoxy(x, y);
    printf("%-*s", tamano - 1, "");
    gotoxy(x, y);
    return leerLinea(destino, tamano);
}

int leerClaveEnPosicion(char *destino, int tamano, int x, int y) {
    int tecla;
    int i = 0;
    gotoxy(x, y);
    printf("%-*s", tamano - 1, "");
    gotoxy(x, y);
    while (i < tamano - 1) {
        tecla = getch();
        if (tecla == 27) {
            destino[0] = '\0';
            return 0;
        }
        if (tecla == 13) {
            break;
        }
        if (tecla == 8) {
            if (i > 0) {
                i--;
                destino[i] = '\0';
                printf("\b \b");
            }
            continue;
        }
        if (tecla == 0 || tecla == 224) {
            getch();
            continue;
        }
        if (tecla >= 32 && tecla <= 126) {
            destino[i] = (char)tecla;
            i++;
            printf("*");
        }
    }
    destino[i] = '\0';
    return 1;
}

int textoVacio(const char *texto) {
    int i;
    for (i = 0; texto[i] != '\0'; i++) {
        if (!isspace((unsigned char)texto[i])) {
            return 0;
        }
    }
    return 1;
}

int soloNumeros(const char *texto) {
    int i;
    if (textoVacio(texto)) {
        return 0;
    }
    for (i = 0; texto[i] != '\0'; i++) {
        if (!isdigit((unsigned char)texto[i])) {
            return 0;
        }
    }
    return 1;
}

int textoANumero(const char *texto, int *valor) {
    if (!soloNumeros(texto)) {
        return 0;
    }
    *valor = atoi(texto);
    return 1;
}

int textoAFloat(const char *texto, float *valor) {
    char copia[40];
    int i;
    int puntos = 0;
    int digitos = 0;

    if (textoVacio(texto) || strlen(texto) >= sizeof(copia)) {
        return 0;
    }

    strcpy(copia, texto);
    for (i = 0; copia[i] != '\0'; i++) {
        if (copia[i] == ',') {
            copia[i] = '.';
        }
        if (copia[i] == '.') {
            puntos++;
            if (puntos > 1) {
                return 0;
            }
        } else if (isdigit((unsigned char)copia[i])) {
            digitos++;
        } else {
            return 0;
        }
    }

    if (digitos == 0) {
        return 0;
    }

    *valor = (float)atof(copia);
    return 1;
}

int confirmar(const char *mensaje) {
    int tecla;
    gotoxy(4, 24);
    printf("%-110s", "");
    gotoxy(4, 24);
    printf("%s", mensaje);
    while (1) {
        tecla = toupper(getch());
        if (tecla == 'S') {
            return 1;
        }
        if (tecla == 'N') {
            return 0;
        }
        if (tecla == 27) {
            return 0;
        }
    }
}

int confirmarSalida(void) {
    return confirmar("Esta seguro que desea salir? (S/N)");
}

int confirmarGuardar(void) {
    return confirmar("Confirmar los datos ingresados? (S/N)");
}

int confirmarEdicion(void) {
    return confirmar("Se perderan los datos anteriores. Desea continuar con la edicion? (S/N)");
}

int confirmarEliminacion(void) {
    return confirmar("ATENCION: Esta accion no se puede deshacer. Desea eliminar este registro definitivamente? (S/N)");
}

int leerOpcion(int x, int y) {
    char buffer[20];
    int valor;
    gotoxy(x, y);
    printf("%-12s", "");
    gotoxy(x, y);
    if (!leerLinea(buffer, sizeof(buffer))) {
        return OPCION_CANCELADA;
    }
    if (!textoANumero(buffer, &valor)) {
        mensajeError("Error de formato: Ingrese solo caracteres numericos.");
        pausaReintentar();
        return OPCION_INVALIDA;
    }
    return valor;
}

int pedirTextoObligatorio(char *destino, int tamano, int x, int y) {
    while (1) {
        if (!leerEnPosicion(destino, tamano, x, y)) {
            return 0;
        }
        if (!textoVacio(destino)) {
            return 1;
        }
        mensajeError("Error: Este campo no puede quedar vacio. Intente de nuevo.");
        pausaReintentar();
        gotoxy(4, 25);
        printf("%-110s", "");
    }
}

int pedirCedula(char *destino, int tamano, int x, int y) {
    while (1) {
        if (!leerEnPosicion(destino, tamano, x, y)) {
            return 0;
        }
        if (textoVacio(destino)) {
            mensajeError("Error: Este campo no puede quedar vacio. Intente de nuevo.");
            pausaReintentar();
        } else if (!soloNumeros(destino)) {
            mensajeError("Error de formato: Ingrese solo caracteres numericos.");
            pausaReintentar();
        } else {
            return 1;
        }
        gotoxy(4, 25);
        printf("%-110s", "");
    }
}

int pedirFloat(float *valor, int x, int y) {
    char buffer[40];
    while (1) {
        if (!leerEnPosicion(buffer, sizeof(buffer), x, y)) {
            return 0;
        }
        if (textoVacio(buffer)) {
            mensajeError("Error: Este campo no puede quedar vacio. Intente de nuevo.");
            pausaReintentar();
        } else if (!textoAFloat(buffer, valor)) {
            mensajeError("Error de formato: Ingrese solo caracteres numericos.");
            pausaReintentar();
        } else {
            return 1;
        }
        gotoxy(4, 25);
        printf("%-110s", "");
    }
}

void mostrarTablaAlumnos(int seccionId, int yInicio) {
    int i;
    int fila = yInicio + 2;
    int hay = 0;

    gotoxy(4, yInicio);
    printf("%-4s %-14s %-25s %-25s", "Nro", "Cedula", "Nombre", "Apellido");

    for (i = 0; i < totalAlumnos; i++) {
        if (alumnos[i].seccionId == seccionId) {
            hay = 1;
            gotoxy(4, fila);
            printf("%-4d %-14s %-25.25s %-25.25s",
                   fila - yInicio - 1,
                   alumnos[i].cedula,
                   alumnos[i].nombre,
                   alumnos[i].apellido);
            fila++;
            if (fila >= 22) {
                gotoxy(4, fila);
                printf("Hay mas alumnos registrados.");
                break;
            }
        }
    }

    if (!hay) {
        gotoxy(4, yInicio + 2);
        printf("Sin alumnos registrados");
    }
}

void mostrarListaEvaluaciones(int indiceSeccion, int yInicio) {
    int i;
    float total = 0.0f;

    gotoxy(4, yInicio);
    printf("%-4s %-45s %-12s", "Nro", "Descripcion", "Ponderacion");

    if (secciones[indiceSeccion].totalEvaluaciones == 0) {
        gotoxy(4, yInicio + 2);
        printf("Sin evaluaciones registradas");
        return;
    }

    for (i = 0; i < secciones[indiceSeccion].totalEvaluaciones; i++) {
        gotoxy(4, yInicio + 2 + i);
        printf("%-4d %-45.45s %8.2f%%",
               i + 1,
               secciones[indiceSeccion].evaluaciones[i].descripcion,
               secciones[indiceSeccion].evaluaciones[i].ponderacion);
        total += secciones[indiceSeccion].evaluaciones[i].ponderacion;
    }
    gotoxy(4, yInicio + 4 + secciones[indiceSeccion].totalEvaluaciones);
    printf("Total acumulado: %.2f%%", total);
}

int contarAlumnosSeccion(int seccionId) {
    int i;
    int total = 0;
    for (i = 0; i < totalAlumnos; i++) {
        if (alumnos[i].seccionId == seccionId) {
            total++;
        }
    }
    return total;
}

int buscarSeccionPorNombre(const char *nombre, int ignorarIndice) {
    int i;
    for (i = 0; i < totalSecciones; i++) {
        if (i != ignorarIndice && strcmp(secciones[i].nombre, nombre) == 0) {
            return i;
        }
    }
    return -1;
}

int buscarAlumnoPorCedula(const char *cedula, int seccionId) {
    int i;
    for (i = 0; i < totalAlumnos; i++) {
        if (alumnos[i].seccionId == seccionId && strcmp(alumnos[i].cedula, cedula) == 0) {
            return i;
        }
    }
    return -1;
}

int indiceSeccionPorId(int id) {
    int i;
    for (i = 0; i < totalSecciones; i++) {
        if (secciones[i].id == id) {
            return i;
        }
    }
    return -1;
}

float sumaPonderaciones(int indiceSeccion, int ignorarIndice) {
    int i;
    float suma = 0.0f;
    for (i = 0; i < secciones[indiceSeccion].totalEvaluaciones; i++) {
        if (i != ignorarIndice) {
            suma += secciones[indiceSeccion].evaluaciones[i].ponderacion;
        }
    }
    return suma;
}

float notaDefinitiva(const Alumno *alumno, const Seccion *seccion) {
    int i;
    float total = 0.0f;
    for (i = 0; i < seccion->totalEvaluaciones; i++) {
        if (alumno->tieneNota[i]) {
            total += alumno->notas[i] * (seccion->evaluaciones[i].ponderacion / 100.0f);
        }
    }
    return total;
}

const char *estadoAlumno(float definitiva) {
    if (definitiva >= NOTA_APROBATORIA) {
        return "Aprobado";
    }
    return "Reprobado";
}

int seleccionarSeccion(void) {
    int i;
    int opcion;

    cabecera("Seleccionar Seccion de Trabajo");
    if (totalSecciones == 0) {
        mensajeError("No hay secciones registradas");
        pausaMenu();
        return -1;
    }

    gotoxy(4, 5);
    printf("%-4s %-35s %-12s %-12s", "Nro", "Seccion", "Alumnos", "Evaluaciones");
    for (i = 0; i < totalSecciones; i++) {
        gotoxy(4, 7 + i);
        printf("%-4d %-35.35s %-12d %-12d",
               i + 1,
               secciones[i].nombre,
               contarAlumnosSeccion(secciones[i].id),
               secciones[i].totalEvaluaciones);
    }
    gotoxy(4, 24);
    printf("Seleccione una seccion (0 regresar): ");
    opcion = leerOpcion(40, 24);
    if (opcion == 0 || opcion == OPCION_CANCELADA || opcion == OPCION_INVALIDA) {
        return -1;
    }
    if (opcion < 1 || opcion > totalSecciones) {
        mensajeError("Seccion invalida.");
        pausaMenu();
        return -1;
    }
    return opcion - 1;
}

int seleccionarAlumno(void) {
    int i;
    int opcion;
    int mapa[MAX_ALUMNOS];
    int total = 0;
    int seccionId;

    if (seccionActiva < 0) {
        mensajeError("Primero debe seleccionar una seccion.");
        pausaMenu();
        return -1;
    }

    seccionId = secciones[seccionActiva].id;
    cabecera("Seleccionar Alumno");
    mostrarTablaAlumnos(seccionId, 5);

    for (i = 0; i < totalAlumnos; i++) {
        if (alumnos[i].seccionId == seccionId) {
            mapa[total] = i;
            total++;
        }
    }

    if (total == 0) {
        pausaMenu();
        return -1;
    }

    gotoxy(4, 24);
    printf("Seleccione alumno (0 regresar): ");
    opcion = leerOpcion(36, 24);
    if (opcion == 0 || opcion == OPCION_CANCELADA || opcion == OPCION_INVALIDA) {
        return -1;
    }
    if (opcion < 1 || opcion > total) {
        mensajeError("Alumno invalido.");
        pausaMenu();
        return -1;
    }
    return mapa[opcion - 1];
}

int seleccionarEvaluacion(int indiceSeccion) {
    int opcion;

    cabecera("Seleccionar Evaluacion");
    mostrarListaEvaluaciones(indiceSeccion, 5);

    if (secciones[indiceSeccion].totalEvaluaciones == 0) {
        pausaMenu();
        return -1;
    }

    gotoxy(4, 24);
    printf("Seleccione evaluacion (0 regresar): ");
    opcion = leerOpcion(39, 24);
    if (opcion == 0 || opcion == OPCION_CANCELADA || opcion == OPCION_INVALIDA) {
        return -1;
    }
    if (opcion < 1 || opcion > secciones[indiceSeccion].totalEvaluaciones) {
        mensajeError("Evaluacion invalida.");
        pausaMenu();
        return -1;
    }
    return opcion - 1;
}

void eliminarAlumnoPorIndice(int indice) {
    int i;
    for (i = indice; i < totalAlumnos - 1; i++) {
        alumnos[i] = alumnos[i + 1];
    }
    totalAlumnos--;
}

void eliminarEvaluacion(int indiceSeccion, int indiceEvaluacion) {
    int i;
    int j;
    int seccionId = secciones[indiceSeccion].id;

    for (i = indiceEvaluacion; i < secciones[indiceSeccion].totalEvaluaciones - 1; i++) {
        secciones[indiceSeccion].evaluaciones[i] = secciones[indiceSeccion].evaluaciones[i + 1];
    }
    secciones[indiceSeccion].totalEvaluaciones--;

    for (i = 0; i < totalAlumnos; i++) {
        if (alumnos[i].seccionId != seccionId) {
            continue;
        }
        for (j = indiceEvaluacion; j < MAX_EVALUACIONES - 1; j++) {
            alumnos[i].notas[j] = alumnos[i].notas[j + 1];
            alumnos[i].tieneNota[j] = alumnos[i].tieneNota[j + 1];
        }
        alumnos[i].notas[MAX_EVALUACIONES - 1] = 0.0f;
        alumnos[i].tieneNota[MAX_EVALUACIONES - 1] = 0;
    }
}

void eliminarSeccionPorIndice(int indiceSeccion) {
    int i;
    int seccionId = secciones[indiceSeccion].id;

    for (i = 0; i < totalAlumnos;) {
        if (alumnos[i].seccionId == seccionId) {
            eliminarAlumnoPorIndice(i);
        } else {
            i++;
        }
    }

    for (i = indiceSeccion; i < totalSecciones - 1; i++) {
        secciones[i] = secciones[i + 1];
    }
    totalSecciones--;

    if (seccionActiva == indiceSeccion) {
        seccionActiva = -1;
    } else if (seccionActiva > indiceSeccion) {
        seccionActiva--;
    }
}

void menuPrincipal(void) {
    int opcion;

    while (1) {
        cabecera("Menu Principal");
        gotoxy(8, 7);
        printf("1. Ingresar");
        gotoxy(8, 9);
        printf("2. Crear Usuario");
        gotoxy(8, 11);
        printf("3. Salir");
        gotoxy(8, 14);
        printf("Opcion: ");
        opcion = leerOpcion(16, 14);

        if (opcion == 1) {
            if (!usuarioPrincipal.creado) {
                mensajeError("Primero debe crear un usuario.");
                pausaMenu();
            } else if (menuIngreso()) {
                menuGestionSecciones();
            }
        } else if (opcion == 2) {
            if (crearUsuario()) {
                if (menuIngreso()) {
                    menuGestionSecciones();
                }
            }
        } else if (opcion == 3) {
            if (confirmarSalida()) {
                break;
            }
        } else if (opcion != OPCION_INVALIDA && opcion != OPCION_CANCELADA) {
            mensajeError("Opcion invalida.");
            pausaMenu();
        }
    }
}

int crearUsuario(void) {
    Usuario nuevo;

    cabecera("Crear Usuario");
    if (usuarioPrincipal.creado) {
        mensajeError("Ya existe un usuario creado.");
        pausaMenu();
        return 0;
    }

    memset(&nuevo, 0, sizeof(nuevo));

    gotoxy(8, 7);
    printf("Nombre:             ______________________________");
    gotoxy(8, 9);
    printf("Apellido:           ______________________________");
    gotoxy(8, 11);
    printf("Nombre de Usuario:  ______________________________");
    gotoxy(8, 13);
    printf("Contrasena:         ______________________________");

    if (!pedirTextoObligatorio(nuevo.nombre, sizeof(nuevo.nombre), 29, 7)) {
        return 0;
    }
    if (!pedirTextoObligatorio(nuevo.apellido, sizeof(nuevo.apellido), 29, 9)) {
        return 0;
    }
    if (!pedirTextoObligatorio(nuevo.usuario, sizeof(nuevo.usuario), 29, 11)) {
        return 0;
    }

    while (1) {
        if (!leerClaveEnPosicion(nuevo.clave, sizeof(nuevo.clave), 29, 13)) {
            return 0;
        }
        if ((int)strlen(nuevo.clave) > 4) {
            break;
        }
        mensajeError("La contrasena debe tener mas de cuatro caracteres.");
        pausaReintentar();
        gotoxy(4, 25);
        printf("%-110s", "");
    }

    gotoxy(8, 17);
    printf("Nombre: %s %s", nuevo.nombre, nuevo.apellido);
    gotoxy(8, 18);
    printf("Usuario: %s", nuevo.usuario);

    if (!confirmarGuardar()) {
        return 0;
    }

    nuevo.creado = 1;
    usuarioPrincipal = nuevo;
    mensajeExito("Usuario creado exitosamente");
    pausaMenu();
    return 1;
}

int menuIngreso(void) {
    char usuario[40];
    char clave[40];
    int opcion;

    while (1) {
        cabecera("Menu de Ingreso");
        gotoxy(8, 8);
        printf("Nombre de Usuario:  ______________________________");
        gotoxy(8, 10);
        printf("Contrasena:         ______________________________");
        gotoxy(8, 14);
        printf("1. Acceder");
        gotoxy(8, 16);
        printf("0. Regresar");

        if (!pedirTextoObligatorio(usuario, sizeof(usuario), 29, 8)) {
            return 0;
        }
        if (!leerClaveEnPosicion(clave, sizeof(clave), 29, 10)) {
            return 0;
        }

        gotoxy(8, 19);
        printf("Opcion: ");
        opcion = leerOpcion(16, 19);

        if (opcion == 0 || opcion == OPCION_CANCELADA) {
            return 0;
        }
        if (opcion == 1) {
            if (strcmp(usuario, usuarioPrincipal.usuario) == 0 &&
                strcmp(clave, usuarioPrincipal.clave) == 0) {
                return 1;
            }
            mensajeError("Uno de los valores no ha coincidido.");
            pausaMenu();
        } else if (opcion != OPCION_INVALIDA && opcion != OPCION_CANCELADA) {
            mensajeError("Opcion invalida.");
            pausaMenu();
        }
    }
}

void menuGestionSecciones(void) {
    int opcion;
    int seleccion;

    while (1) {
        cabecera("Menu de Gestion de Secciones");
        gotoxy(8, 7);
        printf("1. Seleccionar Seccion de Trabajo");
        gotoxy(8, 9);
        printf("2. Crear Nueva Seccion");
        gotoxy(8, 11);
        printf("3. Eliminar Seccion");
        gotoxy(8, 13);
        printf("4. Modificar Seccion");
        gotoxy(8, 15);
        printf("5. Cerrar Sesion");
        if (seccionActiva >= 0) {
            gotoxy(8, 18);
            printf("Seccion activa: %s", secciones[seccionActiva].nombre);
        }
        gotoxy(8, 21);
        printf("Opcion: ");
        opcion = leerOpcion(16, 21);

        if (opcion == OPCION_CANCELADA) {
            if (confirmarSalida()) {
                seccionActiva = -1;
                break;
            }
        } else if (opcion == 1) {
            seleccion = seleccionarSeccion();
            if (seleccion >= 0) {
                seccionActiva = seleccion;
                menuSistema();
            }
        } else if (opcion == 2) {
            crearSeccion();
        } else if (opcion == 3) {
            borrarSeccion();
        } else if (opcion == 4) {
            modificarSeccion();
        } else if (opcion == 5) {
            if (confirmarSalida()) {
                seccionActiva = -1;
                break;
            }
        } else if (opcion != OPCION_INVALIDA && opcion != OPCION_CANCELADA) {
            mensajeError("Opcion invalida.");
            pausaMenu();
        }
    }
}

void crearSeccion(void) {
    Seccion nueva;

    cabecera("Crear Nueva Seccion");
    if (totalSecciones >= MAX_SECCIONES) {
        mensajeError("No hay espacio para mas secciones.");
        pausaMenu();
        return;
    }

    memset(&nueva, 0, sizeof(nueva));
    gotoxy(8, 8);
    printf("Nombre de la seccion: ______________________________");
    if (!pedirTextoObligatorio(nueva.nombre, sizeof(nueva.nombre), 31, 8)) {
        return;
    }

    if (buscarSeccionPorNombre(nueva.nombre, -1) >= 0) {
        mensajeError("Ya existe una seccion con ese nombre exacto.");
        pausaMenu();
        return;
    }

    if (!confirmarGuardar()) {
        return;
    }

    nueva.id = siguienteSeccionId++;
    nueva.totalEvaluaciones = 0;
    secciones[totalSecciones] = nueva;
    seccionActiva = totalSecciones;
    totalSecciones++;
    mensajeExito("Seccion creada exitosamente.");
    pausaMenu();
}

void modificarSeccion(void) {
    char nuevoNombre[60];

    cabecera("Modificar Seccion");
    if (seccionActiva < 0) {
        mensajeError("Primero debe seleccionar una seccion.");
        pausaMenu();
        return;
    }

    gotoxy(8, 7);
    printf("Seccion actual: %s", secciones[seccionActiva].nombre);
    if (!confirmarEdicion()) {
        return;
    }

    gotoxy(8, 11);
    printf("Nuevo nombre: ______________________________");
    if (!pedirTextoObligatorio(nuevoNombre, sizeof(nuevoNombre), 22, 11)) {
        return;
    }

    if (buscarSeccionPorNombre(nuevoNombre, seccionActiva) >= 0) {
        mensajeError("Ya existe una seccion con ese nombre exacto.");
        pausaMenu();
        return;
    }

    strcpy(secciones[seccionActiva].nombre, nuevoNombre);
    mensajeExito("Seccion modificada exitosamente.");
    pausaMenu();
}

void borrarSeccion(void) {
    int indice;

    indice = seleccionarSeccion();
    if (indice < 0) {
        return;
    }

    cabecera("Eliminar Seccion");
    gotoxy(8, 8);
    printf("Seccion: %s", secciones[indice].nombre);
    gotoxy(8, 10);
    printf("Alumnos que se eliminaran: %d", contarAlumnosSeccion(secciones[indice].id));

    if (!confirmarEliminacion()) {
        return;
    }

    eliminarSeccionPorIndice(indice);
    mensajeExito("Seccion eliminada exitosamente.");
    pausaMenu();
}

void menuSistema(void) {
    int opcion;

    while (seccionActiva >= 0) {
        cabecera("Menu del Sistema");
        gotoxy(4, 5);
        printf("Seccion seleccionada: %s", secciones[seccionActiva].nombre);
        gotoxy(8, 8);
        printf("1. Gestion de Alumnos");
        gotoxy(8, 10);
        printf("2. Plan de Evaluacion");
        gotoxy(8, 12);
        printf("3. Gestion Notas");
        gotoxy(8, 14);
        printf("4. Actas");
        gotoxy(8, 16);
        printf("0. Regresar");
        gotoxy(8, 20);
        printf("Opcion: ");
        opcion = leerOpcion(16, 20);

        if (opcion == OPCION_CANCELADA) {
            break;
        } else if (opcion == 1) {
            menuAlumnos();
        } else if (opcion == 2) {
            menuPlanEvaluacion();
        } else if (opcion == 3) {
            menuNotas();
        } else if (opcion == 4) {
            menuActas();
        } else if (opcion == 0) {
            break;
        } else if (opcion != OPCION_INVALIDA && opcion != OPCION_CANCELADA) {
            mensajeError("Opcion invalida.");
            pausaMenu();
        }
    }
}

void menuAlumnos(void) {
    int opcion;

    while (1) {
        cabecera("Menu de Alumnos");
        gotoxy(4, 5);
        printf("Seccion: %s", secciones[seccionActiva].nombre);
        mostrarTablaAlumnos(secciones[seccionActiva].id, 7);
        gotoxy(78, 7);
        printf("1. Registrar");
        gotoxy(78, 9);
        printf("2. Editar");
        gotoxy(78, 11);
        printf("3. Retirar");
        gotoxy(78, 13);
        printf("0. Regresar");
        gotoxy(78, 17);
        printf("Opcion: ");
        opcion = leerOpcion(86, 17);

        if (opcion == OPCION_CANCELADA) {
            break;
        } else if (opcion == 1) {
            registrarAlumno();
        } else if (opcion == 2) {
            editarAlumno();
        } else if (opcion == 3) {
            retirarAlumno();
        } else if (opcion == 0) {
            break;
        } else if (opcion != OPCION_INVALIDA && opcion != OPCION_CANCELADA) {
            mensajeError("Opcion invalida.");
            pausaMenu();
        }
    }
}

void registrarAlumno(void) {
    Alumno nuevo;
    int i;

    cabecera("Registrar Alumno");
    if (totalAlumnos >= MAX_ALUMNOS) {
        mensajeError("No hay espacio para mas alumnos.");
        pausaMenu();
        return;
    }

    memset(&nuevo, 0, sizeof(nuevo));
    nuevo.seccionId = secciones[seccionActiva].id;

    gotoxy(8, 7);
    printf("Cedula:   ____________________");
    gotoxy(8, 9);
    printf("Nombre:   ______________________________");
    gotoxy(8, 11);
    printf("Apellido: ______________________________");

    while (1) {
        if (!pedirCedula(nuevo.cedula, sizeof(nuevo.cedula), 18, 7)) {
            return;
        }
        if (buscarAlumnoPorCedula(nuevo.cedula, nuevo.seccionId) < 0) {
            break;
        }
        mensajeError("Esa cedula ya esta registrada en esta seccion.");
        pausaReintentar();
        gotoxy(4, 25);
        printf("%-110s", "");
    }

    if (!pedirTextoObligatorio(nuevo.nombre, sizeof(nuevo.nombre), 18, 9)) {
        return;
    }
    if (!pedirTextoObligatorio(nuevo.apellido, sizeof(nuevo.apellido), 18, 11)) {
        return;
    }

    gotoxy(8, 15);
    printf("Cedula: %s", nuevo.cedula);
    gotoxy(8, 16);
    printf("Alumno: %s %s", nuevo.nombre, nuevo.apellido);

    if (!confirmarGuardar()) {
        return;
    }

    for (i = 0; i < MAX_EVALUACIONES; i++) {
        nuevo.notas[i] = 0.0f;
        nuevo.tieneNota[i] = 0;
    }

    alumnos[totalAlumnos] = nuevo;
    totalAlumnos++;
    mensajeExito("Alumno registrado exitosamente.");
    pausaMenu();
}

void editarAlumno(void) {
    int indice;
    int opcion;
    Alumno copia;
    char nuevaCedula[20];

    indice = seleccionarAlumno();
    if (indice < 0) {
        return;
    }

    cabecera("Editar Alumno");
    if (!confirmarEdicion()) {
        return;
    }

    copia = alumnos[indice];
    while (1) {
        cabecera("Editar Alumno");
        gotoxy(4, 5);
        printf("Cedula: %s", copia.cedula);
        gotoxy(4, 6);
        printf("Nombre: %s", copia.nombre);
        gotoxy(4, 7);
        printf("Apellido: %s", copia.apellido);
        gotoxy(8, 10);
        printf("1. Editar Cedula");
        gotoxy(8, 12);
        printf("2. Editar Nombre");
        gotoxy(8, 14);
        printf("3. Editar Apellido");
        gotoxy(8, 16);
        printf("4. Guardar");
        gotoxy(8, 18);
        printf("0. Regresar");
        gotoxy(8, 21);
        printf("Opcion: ");
        opcion = leerOpcion(16, 21);

        if (opcion == OPCION_CANCELADA) {
            return;
        } else if (opcion == 1) {
            gotoxy(42, 10);
            printf("Nueva cedula: ____________________");
            while (1) {
                if (!pedirCedula(nuevaCedula, sizeof(nuevaCedula), 57, 10)) {
                    return;
                }
                if (strcmp(nuevaCedula, alumnos[indice].cedula) == 0 ||
                    buscarAlumnoPorCedula(nuevaCedula, copia.seccionId) < 0) {
                    strcpy(copia.cedula, nuevaCedula);
                    break;
                }
                mensajeError("Esa cedula ya esta registrada en esta seccion.");
                pausaReintentar();
            }
        } else if (opcion == 2) {
            gotoxy(42, 12);
            printf("Nuevo nombre: ______________________________");
            if (!pedirTextoObligatorio(copia.nombre, sizeof(copia.nombre), 56, 12)) {
                return;
            }
        } else if (opcion == 3) {
            gotoxy(42, 14);
            printf("Nuevo apellido: ______________________________");
            if (!pedirTextoObligatorio(copia.apellido, sizeof(copia.apellido), 58, 14)) {
                return;
            }
        } else if (opcion == 4) {
            if (confirmarGuardar()) {
                alumnos[indice] = copia;
                mensajeExito("Alumno actualizado exitosamente.");
                pausaMenu();
                return;
            }
        } else if (opcion == 0) {
            return;
        } else if (opcion != OPCION_INVALIDA && opcion != OPCION_CANCELADA) {
            mensajeError("Opcion invalida.");
            pausaMenu();
        }
    }
}

void retirarAlumno(void) {
    int indice;

    indice = seleccionarAlumno();
    if (indice < 0) {
        return;
    }

    cabecera("Retirar Alumno");
    gotoxy(8, 8);
    printf("Cedula: %s", alumnos[indice].cedula);
    gotoxy(8, 10);
    printf("Alumno: %s %s", alumnos[indice].nombre, alumnos[indice].apellido);

    if (!confirmarEliminacion()) {
        return;
    }

    eliminarAlumnoPorIndice(indice);
    mensajeExito("Alumno retirado exitosamente.");
    pausaMenu();
}

void menuPlanEvaluacion(void) {
    int opcion;

    while (1) {
        cabecera("Menu del Plan de Evaluacion");
        gotoxy(4, 5);
        printf("Seccion: %s | Total de Alumnos: %d",
               secciones[seccionActiva].nombre,
               contarAlumnosSeccion(secciones[seccionActiva].id));
        mostrarListaEvaluaciones(seccionActiva, 7);
        gotoxy(78, 7);
        printf("1. Registrar Evaluacion");
        gotoxy(78, 9);
        printf("2. Editar Evaluacion");
        gotoxy(78, 11);
        printf("3. Eliminar Evaluacion");
        gotoxy(78, 13);
        printf("0. Regresar");
        gotoxy(78, 17);
        printf("Opcion: ");
        opcion = leerOpcion(86, 17);

        if (opcion == OPCION_CANCELADA) {
            break;
        } else if (opcion == 1) {
            registrarEvaluacion();
        } else if (opcion == 2) {
            editarEvaluacion();
        } else if (opcion == 3) {
            borrarEvaluacion();
        } else if (opcion == 0) {
            break;
        } else if (opcion != OPCION_INVALIDA && opcion != OPCION_CANCELADA) {
            mensajeError("Opcion invalida.");
            pausaMenu();
        }
    }
}

void registrarEvaluacion(void) {
    Evaluacion nueva;
    float sumaActual;

    cabecera("Registrar Evaluacion");
    if (secciones[seccionActiva].totalEvaluaciones >= MAX_EVALUACIONES) {
        mensajeError("No hay espacio para mas evaluaciones.");
        pausaMenu();
        return;
    }

    gotoxy(8, 7);
    printf("Descripcion: ______________________________");
    gotoxy(8, 9);
    printf("Ponderacion: __________");

    if (!pedirTextoObligatorio(nueva.descripcion, sizeof(nueva.descripcion), 21, 7)) {
        return;
    }
    while (1) {
        if (!pedirFloat(&nueva.ponderacion, 21, 9)) {
            return;
        }
        sumaActual = sumaPonderaciones(seccionActiva, -1);
        if (nueva.ponderacion <= 0.0f) {
            mensajeError("La ponderacion debe ser mayor a 0.");
            pausaReintentar();
        } else if (sumaActual + nueva.ponderacion > 100.0f) {
            mensajeError("La suma de ponderaciones no puede sobrepasar el 100%.");
            pausaReintentar();
        } else {
            break;
        }
        gotoxy(4, 25);
        printf("%-110s", "");
    }

    gotoxy(8, 13);
    printf("Descripcion: %s", nueva.descripcion);
    gotoxy(8, 14);
    printf("Ponderacion: %.2f%%", nueva.ponderacion);

    if (!confirmarGuardar()) {
        return;
    }

    secciones[seccionActiva].evaluaciones[secciones[seccionActiva].totalEvaluaciones] = nueva;
    secciones[seccionActiva].totalEvaluaciones++;
    mensajeExito("Evaluacion registrada exitosamente.");
    pausaMenu();
}

void editarEvaluacion(void) {
    int indiceEvaluacion;
    int opcion;
    Evaluacion copia;
    float nuevaPonderacion;
    float sumaSinActual;

    indiceEvaluacion = seleccionarEvaluacion(seccionActiva);
    if (indiceEvaluacion < 0) {
        return;
    }

    cabecera("Editar Evaluacion");
    if (!confirmarEdicion()) {
        return;
    }

    copia = secciones[seccionActiva].evaluaciones[indiceEvaluacion];
    while (1) {
        cabecera("Editar Evaluacion");
        gotoxy(4, 5);
        printf("Descripcion: %s", copia.descripcion);
        gotoxy(4, 6);
        printf("Ponderacion: %.2f%%", copia.ponderacion);
        gotoxy(8, 10);
        printf("1. Editar Descripcion");
        gotoxy(8, 12);
        printf("2. Editar Ponderacion");
        gotoxy(8, 14);
        printf("3. Guardar");
        gotoxy(8, 16);
        printf("0. Regresar");
        gotoxy(8, 20);
        printf("Opcion: ");
        opcion = leerOpcion(16, 20);

        if (opcion == OPCION_CANCELADA) {
            return;
        } else if (opcion == 1) {
            gotoxy(42, 10);
            printf("Nueva descripcion: ______________________________");
            if (!pedirTextoObligatorio(copia.descripcion, sizeof(copia.descripcion), 61, 10)) {
                return;
            }
        } else if (opcion == 2) {
            gotoxy(42, 12);
            printf("Nueva ponderacion: __________");
            while (1) {
                if (!pedirFloat(&nuevaPonderacion, 62, 12)) {
                    return;
                }
                sumaSinActual = sumaPonderaciones(seccionActiva, indiceEvaluacion);
                if (nuevaPonderacion <= 0.0f) {
                    mensajeError("La ponderacion debe ser mayor a 0.");
                    pausaReintentar();
                } else if (sumaSinActual + nuevaPonderacion > 100.0f) {
                    mensajeError("La suma de ponderaciones no puede sobrepasar el 100%.");
                    pausaReintentar();
                } else {
                    copia.ponderacion = nuevaPonderacion;
                    break;
                }
                gotoxy(4, 25);
                printf("%-110s", "");
            }
        } else if (opcion == 3) {
            if (confirmarGuardar()) {
                secciones[seccionActiva].evaluaciones[indiceEvaluacion] = copia;
                mensajeExito("Evaluacion actualizada exitosamente.");
                pausaMenu();
                return;
            }
        } else if (opcion == 0) {
            return;
        } else if (opcion != OPCION_INVALIDA && opcion != OPCION_CANCELADA) {
            mensajeError("Opcion invalida.");
            pausaMenu();
        }
    }
}

void borrarEvaluacion(void) {
    int indiceEvaluacion;

    indiceEvaluacion = seleccionarEvaluacion(seccionActiva);
    if (indiceEvaluacion < 0) {
        return;
    }

    cabecera("Eliminar Evaluacion");
    gotoxy(8, 8);
    printf("Evaluacion: %s", secciones[seccionActiva].evaluaciones[indiceEvaluacion].descripcion);
    gotoxy(8, 10);
    printf("Tambien se borraran las notas guardadas en esta evaluacion.");

    if (!confirmarEliminacion()) {
        return;
    }

    eliminarEvaluacion(seccionActiva, indiceEvaluacion);
    mensajeExito("Evaluacion eliminada exitosamente.");
    pausaMenu();
}

void menuNotas(void) {
    int opcion;

    while (1) {
        cabecera("Menu de Notas");
        gotoxy(4, 5);
        printf("Seccion: %s", secciones[seccionActiva].nombre);
        mostrarTablaAlumnos(secciones[seccionActiva].id, 7);
        gotoxy(78, 7);
        printf("1. Cargar / Editar Calificaciones");
        gotoxy(78, 9);
        printf("2. Ver Resumen del Alumno");
        gotoxy(78, 11);
        printf("0. Regresar");
        gotoxy(78, 15);
        printf("Opcion: ");
        opcion = leerOpcion(86, 15);

        if (opcion == OPCION_CANCELADA) {
            break;
        } else if (opcion == 1) {
            cargarEditarCalificacion();
        } else if (opcion == 2) {
            verResumenAlumno();
        } else if (opcion == 0) {
            break;
        } else if (opcion != OPCION_INVALIDA && opcion != OPCION_CANCELADA) {
            mensajeError("Opcion invalida.");
            pausaMenu();
        }
    }
}

void cargarEditarCalificacion(void) {
    int indiceAlumno;
    int indiceEvaluacion;
    float nota;

    if (secciones[seccionActiva].totalEvaluaciones == 0) {
        mensajeError("Primero debe registrar evaluaciones en el plan.");
        pausaMenu();
        return;
    }

    indiceAlumno = seleccionarAlumno();
    if (indiceAlumno < 0) {
        return;
    }

    indiceEvaluacion = seleccionarEvaluacion(seccionActiva);
    if (indiceEvaluacion < 0) {
        return;
    }

    cabecera("Ingresar Nota");
    gotoxy(8, 7);
    printf("Alumno: %s %s", alumnos[indiceAlumno].nombre, alumnos[indiceAlumno].apellido);
    gotoxy(8, 9);
    printf("Evaluacion: %s", secciones[seccionActiva].evaluaciones[indiceEvaluacion].descripcion);
    gotoxy(8, 11);
    printf("Nota de 0 a 20: __________");

    while (1) {
        if (!pedirFloat(&nota, 24, 11)) {
            return;
        }
        if (nota >= NOTA_MINIMA && nota <= NOTA_MAXIMA) {
            break;
        }
        mensajeError("La nota debe estar dentro de la escala permitida: 0 a 20.");
        pausaReintentar();
        gotoxy(4, 25);
        printf("%-110s", "");
    }

    gotoxy(8, 15);
    printf("Nota ingresada: %.2f", nota);

    if (!confirmarGuardar()) {
        return;
    }

    alumnos[indiceAlumno].notas[indiceEvaluacion] = nota;
    alumnos[indiceAlumno].tieneNota[indiceEvaluacion] = 1;
    mensajeExito("Nota guardada exitosamente.");
    pausaMenu();
}

void verResumenAlumno(void) {
    int indiceAlumno;
    int i;
    float definitiva;

    indiceAlumno = seleccionarAlumno();
    if (indiceAlumno < 0) {
        return;
    }

    cabecera("Resumen del Alumno");
    gotoxy(4, 5);
    printf("Alumno: %s %s", alumnos[indiceAlumno].nombre, alumnos[indiceAlumno].apellido);
    gotoxy(4, 6);
    printf("Cedula: %s", alumnos[indiceAlumno].cedula);
    gotoxy(4, 8);
    printf("%-4s %-35s %-12s %-12s", "Nro", "Evaluacion", "Peso", "Nota");

    for (i = 0; i < secciones[seccionActiva].totalEvaluaciones; i++) {
        gotoxy(4, 10 + i);
        printf("%-4d %-35.35s %8.2f%% ",
               i + 1,
               secciones[seccionActiva].evaluaciones[i].descripcion,
               secciones[seccionActiva].evaluaciones[i].ponderacion);
        if (alumnos[indiceAlumno].tieneNota[i]) {
            printf("%8.2f", alumnos[indiceAlumno].notas[i]);
        } else {
            printf("%8s", "S/N");
        }
    }

    definitiva = notaDefinitiva(&alumnos[indiceAlumno], &secciones[seccionActiva]);
    gotoxy(4, 23);
    printf("Nota definitiva: %.2f | Estado: %s", definitiva, estadoAlumno(definitiva));
    pausaMenu();
}

void menuActas(void) {
    int opcion;

    while (1) {
        cabecera("Menu de Actas");
        gotoxy(8, 8);
        printf("1. Visualizar Acta General en Pantalla");
        gotoxy(8, 10);
        printf("0. Regresar");
        gotoxy(8, 14);
        printf("Opcion: ");
        opcion = leerOpcion(16, 14);

        if (opcion == OPCION_CANCELADA) {
            break;
        } else if (opcion == 1) {
            visualizarActa();
        } else if (opcion == 0) {
            break;
        } else if (opcion != OPCION_INVALIDA && opcion != OPCION_CANCELADA) {
            mensajeError("Opcion invalida.");
            pausaMenu();
        }
    }
}

void visualizarActa(void) {
    int i;
    int fila = 9;
    int hay = 0;
    float definitiva;
    int seccionId = secciones[seccionActiva].id;

    cabecera("Acta General");
    gotoxy(4, 5);
    printf("Seccion: %s", secciones[seccionActiva].nombre);
    gotoxy(4, 7);
    printf("%-14s %-25s %-25s %-12s %-12s", "Cedula", "Nombre", "Apellido", "Definitiva", "Estado");

    for (i = 0; i < totalAlumnos; i++) {
        if (alumnos[i].seccionId == seccionId) {
            hay = 1;
            definitiva = notaDefinitiva(&alumnos[i], &secciones[seccionActiva]);
            gotoxy(4, fila);
            printf("%-14s %-25.25s %-25.25s %-12.2f %-12s",
                   alumnos[i].cedula,
                   alumnos[i].nombre,
                   alumnos[i].apellido,
                   definitiva,
                   estadoAlumno(definitiva));
            fila++;
            if (fila >= 24) {
                gotoxy(4, fila);
                printf("Hay mas alumnos registrados.");
                break;
            }
        }
    }

    if (!hay) {
        gotoxy(4, 10);
        printf("Sin alumnos registrados");
    }

    pausaMenu();
}

int main(void) {
    configurarConsola();
    memset(&usuarioPrincipal, 0, sizeof(usuarioPrincipal));
    menuPrincipal();
    limpiarPantalla();
    color(7);
    return 0;
}
