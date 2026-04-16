#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <conio.h>
#include <windows.h>
#include <shellapi.h>

#define MAX_DOCENTES 30
#define MAX_MATERIAS 40
#define MAX_ALUMNOS 400
#define MAX_EVALS 5

#define ARCHIVO_DATOS "siga_sencillo.dat"
#define ARCHIVO_MAGIC "SIGA3"
#define CARPETA_IMPRESIONES "impresiones"
#define NOTA_MAXIMA 100.0f
#define NOTA_APROBATORIA 50.0f

#define ANCHO_PANTALLA 120
#define ALTO_PANTALLA 30
#define ANCHO_CUADRO_PRINCIPAL 118
#define ALTO_CUADRO_PRINCIPAL 29

#define COLOR_NORMAL 15
#define COLOR_TITULO 11
#define COLOR_EXITO 10
#define COLOR_ERROR 12
#define COLOR_BARRA 10
#define COLOR_SELECCION 240

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
    char evalTipos[MAX_EVALS][30];
    float ponderaciones[MAX_EVALS];
    int totalEvaluaciones;
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

int loginUsuario(void);
void menuPrincipal(void);
void menuUsuario(void);
void menuGestionAlumnos(void);
void menuGestionAcademico(void);

void usuarioCrearCuenta(void);
int elegirMateriaDelDocente(void);
int elegirEvaluacionMateria(int posMateria);
void docenteRegistrarAlumno(void);
void docenteListarAlumnos(void);
void docenteEditarAlumno(void);
void docenteRetirarAlumno(void);
void docentePlanEvaluacion(void);
void docenteVerNotas(void);
void docenteCargarNotas(void);
void docenteGenerarActa(void);
void docenteImprimirActa(void);
void docenteImprimirBoleta(void);
void verActaMateria(int materiaId, int modoAdmin);
int prepararCarpetaImpresiones(void);
int generarArchivoActa(int materiaId, char ruta[MAX_PATH]);
int generarArchivoBoleta(int materiaId, int posAlumno, char ruta[MAX_PATH]);
int imprimirArchivoTexto(const char *ruta);

void configurarConsola(void) {
    char comando[60];
    snprintf(comando, sizeof(comando), "mode con: cols=%d lines=%d", ANCHO_PANTALLA, ALTO_PANTALLA);
    system(comando);
    SetConsoleTitle("SiGA Sencillo - Terminal");
    ocultarCursor();
}

void establecerFondo(void) {
    system("color 0F");
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
    color(COLOR_NORMAL);
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
            color(COLOR_NORMAL);
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
            color(COLOR_NORMAL);
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
    color(COLOR_NORMAL);
}

void mostrarError(const char *mensaje, int y) {
    color(COLOR_ERROR);
    centrarTexto(mensaje, y);
    color(COLOR_NORMAL);
}

void mostrarExito(const char *mensaje, int y) {
    color(COLOR_EXITO);
    centrarTexto(mensaje, y);
    color(COLOR_NORMAL);
}

void pausa(void) {
    color(COLOR_NORMAL);
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
        color(COLOR_BARRA);
        printf("%c", 219);
        Sleep(8);
    }
    color(COLOR_NORMAL);
}

void obtenerFecha(char fecha[20]) {
    time_t ahora = time(NULL);
    struct tm *tiempo = localtime(&ahora);
    strftime(fecha, 20, "%d/%m/%Y", tiempo);
}

void leerTexto(char *texto, int tamano, int x, int y) {
    activarCursor();
    color(COLOR_NORMAL);
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
                color(COLOR_SELECCION);
                printf(" > %-*s ", ancho - 4, opciones[i]);
            } else {
                color(COLOR_NORMAL);
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
    int i;
    int alumnosActivos = 0;

    for (i = 0; i < totalAlumnos; i++) {
        if (!alumnos[i].retirado) alumnosActivos++;
    }

    establecerFondo();
    limpiarPantalla();
    cuadroPrincipal();
    dibujarCuadro(11, 2, 99, 3);
    mostrarMensajeEnCuadro(titulo, 11, 3, 99, COLOR_TITULO);
    obtenerFecha(fecha);
    gotoxy(88, 6);
    printf("Fecha: %s", fecha);
    gotoxy(8, 6);
    printf("Usuarios: %d  Materias: %d  Alumnos activos: %d", totalDocentes, totalMaterias, alumnosActivos);
}

void colocarPlanDefecto(Materia *materia) {
    strcpy(materia->evalNombres[0], "Parcial 1");
    strcpy(materia->evalNombres[1], "Taller");
    strcpy(materia->evalNombres[2], "Proyecto");
    strcpy(materia->evalTipos[0], "Examen");
    strcpy(materia->evalTipos[1], "Practica");
    strcpy(materia->evalTipos[2], "Proyecto");
    materia->ponderaciones[0] = 40.0f;
    materia->ponderaciones[1] = 30.0f;
    materia->ponderaciones[2] = 30.0f;
    materia->totalEvaluaciones = 3;
    for (int i = 3; i < MAX_EVALS; i++) {
        materia->evalNombres[i][0] = '\0';
        materia->evalTipos[i][0] = '\0';
        materia->ponderaciones[i] = 0.0f;
    }
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

    if (strncmp(magic, ARCHIVO_MAGIC, 5) != 0) {
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
    char magic[8] = ARCHIVO_MAGIC;

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
    int totalEvaluaciones = materia->totalEvaluaciones;
    float definitiva = 0.0f;
    *completo = 1;

    if (totalEvaluaciones <= 0 || totalEvaluaciones > MAX_EVALS) {
        totalEvaluaciones = 3;
    }

    if (alumno->retirado) {
        *completo = 1;
        return 0.0f;
    }

    for (i = 0; i < totalEvaluaciones; i++) {
        if (!alumno->tieneNotas[i]) {
            *completo = 0;
            continue;
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
    if (definitiva >= NOTA_APROBATORIA) return "APROBADO";
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

int loginUsuario(void) {
    char usuario[30], clave[30];
    int pos;

    pantallaBase("LOGIN DE USUARIO");
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
        mostrarBarraCarga("Entrando al panel de usuario...");
        return 1;
    }

    mostrarError("Usuario o contrasena incorrectos.", 22);
    pausa();
    return 0;
}

void menuPrincipal(void) {
    const char *opciones[] = {
        "Iniciar sesion",
        "Crear usuario",
        "Salir"
    };
    int opcion;

    while (1) {
        if (totalDocentes == 0) {
            pantallaBase("CREAR PRIMER USUARIO");
            dibujarCuadro(24, 10, 72, 8);
            mostrarMensajeEnCuadro("No hay usuarios registrados. Cree el primer usuario.", 24, 13, 72, COLOR_NORMAL);
            pausa();
            usuarioCrearCuenta();
            continue;
        }

        pantallaBase("SISTEMA DE GESTION ACADEMICA SENCILLO");
        dibujarCuadro(37, 10, 46, 10);
        mostrarMensajeEnCuadro("Seleccione con flechas y ENTER", 37, 11, 46, COLOR_NORMAL);
        opcion = menuFlechas(opciones, 3, 42, 14, 36);

        if (opcion == 0) {
            if (loginUsuario()) menuUsuario();
        } else if (opcion == 1) {
            usuarioCrearCuenta();
        } else {
            guardarDatos();
            break;
        }
    }
}

void menuUsuario(void) {
    const char *opciones[] = {
        "Gestion de alumnos",
        "Gestion academico",
        "Guardar datos",
        "Cerrar sesion"
    };
    char titulo[140];
    char materiaNombre[70] = "Sin materia";
    int i;
    int opcion;

    while (1) {
        for (i = 0; i < totalMaterias; i++) {
            if (materias[i].docenteId == docentes[docenteActual].id) {
                strcpy(materiaNombre, materias[i].nombre);
                break;
            }
        }

        snprintf(titulo, sizeof(titulo), "PANEL DE USUARIO - %s %s", docentes[docenteActual].nombres, docentes[docenteActual].apellidos);
        pantallaBase(titulo);
        gotoxy(8, 7);
        printf("Materia: %s", materiaNombre);
        dibujarCuadro(31, 9, 58, 12);
        opcion = menuFlechas(opciones, 4, 36, 11, 48);

        if (opcion == 0) menuGestionAlumnos();
        else if (opcion == 1) menuGestionAcademico();
        else if (opcion == 2) {
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

void menuGestionAlumnos(void) {
    const char *opciones[] = {
        "Registrar alumno",
        "Listado de alumnos",
        "Editar datos de alumno",
        "Retirar alumno",
        "Volver"
    };
    int opcion;

    while (1) {
        pantallaBase("GESTION DE ALUMNOS");
        dibujarCuadro(29, 9, 62, 14);
        opcion = menuFlechas(opciones, 5, 34, 11, 52);

        if (opcion == 0) docenteRegistrarAlumno();
        else if (opcion == 1) docenteListarAlumnos();
        else if (opcion == 2) docenteEditarAlumno();
        else if (opcion == 3) docenteRetirarAlumno();
        else break;
    }
}

void menuGestionAcademico(void) {
    const char *opciones[] = {
        "Plan de estudio",
        "Ver notas",
        "Cargar nota",
        "Ver acta",
        "Imprimir acta",
        "Imprimir boleta",
        "Volver"
    };
    int opcion;

    while (1) {
        pantallaBase("GESTION ACADEMICO");
        dibujarCuadro(29, 8, 62, 18);
        opcion = menuFlechas(opciones, 7, 34, 10, 52);

        if (opcion == 0) docentePlanEvaluacion();
        else if (opcion == 1) docenteVerNotas();
        else if (opcion == 2) docenteCargarNotas();
        else if (opcion == 3) docenteGenerarActa();
        else if (opcion == 4) docenteImprimirActa();
        else if (opcion == 5) docenteImprimirBoleta();
        else break;
    }
}

void usuarioCrearCuenta(void) {
    Docente nuevo;
    Materia nuevaMateria;
    char buffer[130];
    char materiaNombre[60];

    memset(&nuevo, 0, sizeof(nuevo));
    memset(&nuevaMateria, 0, sizeof(nuevaMateria));

    pantallaBase("CREAR USUARIO");
    if (totalDocentes >= MAX_DOCENTES) {
        mostrarError("No hay espacio para mas usuarios.", 23);
        pausa();
        return;
    }
    if (totalMaterias >= MAX_MATERIAS) {
        mostrarError("No hay espacio para mas materias.", 23);
        pausa();
        return;
    }

    dibujarCuadro(18, 7, 84, 18);

    gotoxy(24, 9); printf("Nombre: ");
    leerTexto(nuevo.nombres, sizeof(nuevo.nombres), 45, 9);
    if (!validarNombre(nuevo.nombres)) {
        mostrarError("Nombre invalido.", 25);
        pausa();
        return;
    }
    capitalizar(nuevo.nombres);

    gotoxy(24, 11); printf("Apellido: ");
    leerTexto(nuevo.apellidos, sizeof(nuevo.apellidos), 45, 11);
    if (!validarNombre(nuevo.apellidos)) {
        mostrarError("Apellido invalido.", 25);
        pausa();
        return;
    }
    capitalizar(nuevo.apellidos);

    gotoxy(24, 13); printf("Usuario: ");
    leerTexto(nuevo.usuario, sizeof(nuevo.usuario), 45, 13);
    if (strlen(nuevo.usuario) < 3 || buscarDocentePorUsuario(nuevo.usuario) >= 0) {
        mostrarError("Usuario invalido o ya existe.", 25);
        pausa();
        return;
    }

    gotoxy(24, 15); printf("Contrasena: ");
    leerClave(nuevo.clave, sizeof(nuevo.clave), 45, 15);
    if (strlen(nuevo.clave) < 4) {
        mostrarError("La contrasena debe tener minimo 4 caracteres.", 25);
        pausa();
        return;
    }

    gotoxy(24, 17); printf("Materia a impartir: ");
    leerTexto(materiaNombre, sizeof(materiaNombre), 45, 17);
    if (strlen(materiaNombre) < 3) {
        mostrarError("Nombre de materia invalido.", 25);
        pausa();
        return;
    }
    capitalizar(materiaNombre);

    nuevo.id = siguienteIdDocente();
    strcpy(nuevo.cedula, "N/A");
    nuevo.activo = 1;
    docentes[totalDocentes++] = nuevo;

    nuevaMateria.id = siguienteIdMateria();
    snprintf(nuevaMateria.codigo, sizeof(nuevaMateria.codigo), "MAT-%03d", nuevaMateria.id);
    strcpy(nuevaMateria.nombre, materiaNombre);
    strcpy(nuevaMateria.periodo, "Actual");
    nuevaMateria.docenteId = nuevo.id;
    nuevaMateria.actaGenerada = 0;
    colocarPlanDefecto(&nuevaMateria);
    materias[totalMaterias++] = nuevaMateria;

    guardarDatos();

    snprintf(buffer, sizeof(buffer), "Usuario creado: %s | Materia: %s", nuevo.usuario, nuevaMateria.nombre);
    mostrarExito(buffer, 25);
    pausa();
}

int elegirMateriaDelDocente(void) {
    int i;
    int docenteId = docentes[docenteActual].id;

    for (i = 0; i < totalMaterias; i++) {
        if (materias[i].docenteId == docenteId) {
            return materias[i].id;
        }
    }

    mostrarError("Este usuario no tiene materia registrada.", 22);
    pausa();
    return -1;
}

int elegirEvaluacionMateria(int posMateria) {
    int i;
    int totalEvaluaciones;
    int opcion;

    if (posMateria < 0) return -1;

    totalEvaluaciones = materias[posMateria].totalEvaluaciones;
    if (totalEvaluaciones <= 0 || totalEvaluaciones > MAX_EVALS) totalEvaluaciones = 3;

    gotoxy(22, 12);
    printf("Evaluaciones del plan:");
    for (i = 0; i < totalEvaluaciones; i++) {
        gotoxy(24, 14 + i);
        printf("%d. %-22.22s %-14.14s %.2f%%",
               i + 1,
               materias[posMateria].evalNombres[i],
               materias[posMateria].evalTipos[i],
               materias[posMateria].ponderaciones[i]);
    }

    gotoxy(22, 20);
    printf("Numero de evaluacion: ");
    opcion = leerEntero(45, 20);
    if (opcion < 1 || opcion > totalEvaluaciones) {
        mostrarError("Evaluacion invalida.", 25);
        pausa();
        return -1;
    }

    return opcion - 1;
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
    int i, j;
    int totalEvaluaciones;
    int y = 12;
    int completo;
    float definitiva;

    materiaId = elegirMateriaDelDocente();
    if (materiaId < 0) return;
    posMateria = buscarMateriaPorId(materiaId);
    totalEvaluaciones = materias[posMateria].totalEvaluaciones;
    if (totalEvaluaciones <= 0 || totalEvaluaciones > MAX_EVALS) totalEvaluaciones = 3;

    pantallaBase("LISTADO DE ALUMNOS");
    dibujarCuadro(4, 8, 112, 17);
    gotoxy(8, 9);
    printf("Materia: %s", materias[posMateria].nombre);
    gotoxy(8, 10);
    printf("CEDULA       NOMBRES          APELLIDOS        ");
    for (j = 0; j < totalEvaluaciones; j++) {
        printf("E%-2d    ", j + 1);
    }
    printf("PROM.   CONDICION");

    for (i = 0; i < totalAlumnos && y < 24; i++) {
        if (alumnos[i].materiaId == materiaId) {
            definitiva = calcularDefinitivaAlumno(&alumnos[i], &materias[posMateria], &completo);
            gotoxy(8, y);
            printf("%-12s %-16.16s %-16.16s",
                   alumnos[i].cedula,
                   alumnos[i].nombres,
                   alumnos[i].apellidos);
            for (j = 0; j < totalEvaluaciones; j++) {
                if (alumnos[i].tieneNotas[j]) printf(" %5.2f ", alumnos[i].notas[j]);
                else printf("  --   ");
            }
            (void) completo;
            printf(" %6.2f ", definitiva);
            printf("%s", condicionAlumno(&alumnos[i], &materias[posMateria]));
            y++;
        }
    }

    if (y == 12) {
        mostrarError("No hay alumnos registrados en esta materia.", 18);
    }
    pausa();
}

void docenteEditarAlumno(void) {
    int materiaId;
    int posMateria;
    int posAlumno;
    char cedula[20];
    char nuevoValor[50];

    materiaId = elegirMateriaDelDocente();
    if (materiaId < 0) return;
    posMateria = buscarMateriaPorId(materiaId);

    pantallaBase("EDITAR ALUMNO");
    dibujarCuadro(18, 7, 84, 18);
    gotoxy(24, 9);
    printf("Materia: %s", materias[posMateria].nombre);
    gotoxy(24, 11);
    printf("Cedula del alumno: ");
    leerTexto(cedula, sizeof(cedula), 48, 11);

    posAlumno = buscarAlumnoPorCedulaMateria(cedula, materiaId);
    if (posAlumno < 0) {
        mostrarError("Alumno no encontrado.", 25);
        pausa();
        return;
    }

    gotoxy(24, 13);
    printf("Actual: %s | %s %s", alumnos[posAlumno].cedula, alumnos[posAlumno].nombres, alumnos[posAlumno].apellidos);
    gotoxy(24, 15);
    printf("Nueva cedula (ENTER conserva): ");
    leerTexto(nuevoValor, sizeof(nuevoValor), 57, 15);
    if (strlen(nuevoValor) > 0) {
        if (!validarCedula(nuevoValor)) {
            mostrarError("Cedula invalida.", 25);
            pausa();
            return;
        }
        if (strcmp(nuevoValor, alumnos[posAlumno].cedula) != 0 &&
            buscarAlumnoPorCedulaMateria(nuevoValor, materiaId) >= 0) {
            mostrarError("Ya existe otro alumno con esa cedula.", 25);
            pausa();
            return;
        }
        strcpy(alumnos[posAlumno].cedula, nuevoValor);
    }

    gotoxy(24, 17);
    printf("Nuevo nombre (ENTER conserva): ");
    leerTexto(nuevoValor, sizeof(nuevoValor), 57, 17);
    if (strlen(nuevoValor) > 0) {
        if (!validarNombre(nuevoValor)) {
            mostrarError("Nombre invalido.", 25);
            pausa();
            return;
        }
        capitalizar(nuevoValor);
        strcpy(alumnos[posAlumno].nombres, nuevoValor);
    }

    gotoxy(24, 19);
    printf("Nuevo apellido (ENTER conserva): ");
    leerTexto(nuevoValor, sizeof(nuevoValor), 59, 19);
    if (strlen(nuevoValor) > 0) {
        if (!validarNombre(nuevoValor)) {
            mostrarError("Apellido invalido.", 25);
            pausa();
            return;
        }
        capitalizar(nuevoValor);
        strcpy(alumnos[posAlumno].apellidos, nuevoValor);
    }

    materias[posMateria].actaGenerada = 0;
    guardarDatos();
    mostrarExito("Datos del alumno actualizados.", 25);
    pausa();
}

void docenteRetirarAlumno(void) {
    int materiaId;
    int posMateria;
    int posAlumno;
    char cedula[20];

    materiaId = elegirMateriaDelDocente();
    if (materiaId < 0) return;
    posMateria = buscarMateriaPorId(materiaId);

    pantallaBase("RETIRAR ALUMNO");
    dibujarCuadro(22, 9, 76, 12);
    gotoxy(28, 11);
    printf("Materia: %s", materias[posMateria].nombre);
    gotoxy(28, 13);
    printf("Cedula del alumno: ");
    leerTexto(cedula, sizeof(cedula), 50, 13);

    posAlumno = buscarAlumnoPorCedulaMateria(cedula, materiaId);
    if (posAlumno < 0) {
        mostrarError("Alumno no encontrado.", 24);
        pausa();
        return;
    }

    alumnos[posAlumno].retirado = 1;
    materias[posMateria].actaGenerada = 0;
    guardarDatos();
    mostrarExito("Alumno marcado como retirado.", 24);
    pausa();
}

void docentePlanEvaluacion(void) {
    int materiaId;
    int posMateria;
    int i, j;
    int hayNotas = 0;
    int cantidad;
    float suma = 0.0f;
    float diferencia;
    char nombreEval[30];
    char tipoEval[30];
    float pesoEval[MAX_EVALS];
    char nombresEval[MAX_EVALS][30];
    char tiposEval[MAX_EVALS][30];

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

    pantallaBase("PLAN DE ESTUDIO");
    dibujarCuadro(15, 8, 90, 19);
    gotoxy(22, 10);
    printf("Materia: %s", materias[posMateria].nombre);

    if (hayNotas) {
        gotoxy(22, 12);
        printf("El plan no se puede editar porque ya existen notas cargadas.");
        gotoxy(22, 14);
        printf("Plan actual:");
        cantidad = materias[posMateria].totalEvaluaciones;
        if (cantidad <= 0 || cantidad > MAX_EVALS) cantidad = 3;
        for (i = 0; i < cantidad; i++) {
            gotoxy(24, 16 + i);
            printf("%d. %-18s %-14s %.2f%%",
                   i + 1,
                   materias[posMateria].evalNombres[i],
                   materias[posMateria].evalTipos[i],
                   materias[posMateria].ponderaciones[i]);
        }
        pausa();
        return;
    }

    gotoxy(22, 12);
    printf("Cantidad de evaluaciones (1-%d): ", MAX_EVALS);
    cantidad = leerEntero(57, 12);
    if (cantidad < 1 || cantidad > MAX_EVALS) {
        mostrarError("Cantidad de evaluaciones invalida.", 25);
        pausa();
        return;
    }

    gotoxy(22, 14);
    printf("Nombre, tipo y ponderacion. La suma debe ser 100%%.");
    for (i = 0; i < cantidad; i++) {
        gotoxy(22, 16 + i * 2);
        printf("Nombre eval %d: ", i + 1);
        leerTexto(nombreEval, sizeof(nombreEval), 38, 16 + i * 2);
        if (strlen(nombreEval) == 0) {
            snprintf(nombreEval, sizeof(nombreEval), "Evaluacion %d", i + 1);
        }
        gotoxy(52, 16 + i * 2);
        printf("Tipo: ");
        leerTexto(tipoEval, sizeof(tipoEval), 58, 16 + i * 2);
        if (strlen(tipoEval) == 0) {
            strcpy(tipoEval, "General");
        }
        gotoxy(77, 16 + i * 2);
        printf("Peso: ");
        pesoEval[i] = leerFloat(83, 16 + i * 2);
        if (pesoEval[i] <= 0.0f) {
            mostrarError("Cada ponderacion debe ser mayor que cero.", 25);
            pausa();
            return;
        }
        capitalizar(nombreEval);
        capitalizar(tipoEval);
        strcpy(nombresEval[i], nombreEval);
        strcpy(tiposEval[i], tipoEval);
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
        if (i < cantidad) {
            strcpy(materias[posMateria].evalNombres[i], nombresEval[i]);
            strcpy(materias[posMateria].evalTipos[i], tiposEval[i]);
            materias[posMateria].ponderaciones[i] = pesoEval[i];
        } else {
            materias[posMateria].evalNombres[i][0] = '\0';
            materias[posMateria].evalTipos[i][0] = '\0';
            materias[posMateria].ponderaciones[i] = 0.0f;
        }
    }
    materias[posMateria].totalEvaluaciones = cantidad;

    guardarDatos();
    mostrarExito("Plan de estudio guardado correctamente.", 25);
    pausa();
}

void docenteVerNotas(void) {
    docenteListarAlumnos();
}

void docenteCargarNotas(void) {
    int materiaId;
    int posMateria;
    int posAlumno;
    int evaluacion;
    char cedula[20];
    float nota;

    materiaId = elegirMateriaDelDocente();
    if (materiaId < 0) return;
    posMateria = buscarMateriaPorId(materiaId);

    pantallaBase("CARGA DE UNA NOTA");
    dibujarCuadro(15, 8, 90, 19);
    gotoxy(22, 10);
    printf("Materia: %s", materias[posMateria].nombre);
    evaluacion = elegirEvaluacionMateria(posMateria);
    if (evaluacion < 0) return;

    gotoxy(22, 22);
    printf("Cedula del alumno: ");
    leerTexto(cedula, sizeof(cedula), 43, 22);

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

    gotoxy(22, 24);
    printf("Alumno: %s %s", alumnos[posAlumno].nombres, alumnos[posAlumno].apellidos);
    gotoxy(22, 25);
    printf("%s / %s - nota 0 a 100: ",
           materias[posMateria].evalNombres[evaluacion],
           materias[posMateria].evalTipos[evaluacion]);
    nota = leerFloat(58, 25);
    if (nota < 0.0f || nota > NOTA_MAXIMA) {
        mostrarError("La nota debe estar entre 0 y 100.", 26);
        pausa();
        return;
    }

    alumnos[posAlumno].notas[evaluacion] = nota;
    alumnos[posAlumno].tieneNotas[evaluacion] = 1;
    materias[posMateria].actaGenerada = 0;
    guardarDatos();
    mostrarExito("Nota guardada. Si ya existia, quedo corregida.", 26);
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

void docenteImprimirActa(void) {
    int materiaId;
    char ruta[MAX_PATH];

    materiaId = elegirMateriaDelDocente();
    if (materiaId < 0) return;

    if (!generarArchivoActa(materiaId, ruta)) {
        mostrarError("No se pudo generar el archivo del acta.", 25);
        pausa();
        return;
    }

    if (imprimirArchivoTexto(ruta)) {
        mostrarExito("Acta enviada a imprimir con ShellAPI.", 25);
    } else {
        mostrarError("No se pudo enviar a imprimir. Revise impresora o app .txt.", 25);
    }
    gotoxy(8, 26);
    printf("Archivo generado: %s", ruta);
    pausa();
}

void docenteImprimirBoleta(void) {
    int materiaId;
    int posAlumno;
    char cedula[20];
    char ruta[MAX_PATH];

    materiaId = elegirMateriaDelDocente();
    if (materiaId < 0) return;

    pantallaBase("IMPRIMIR BOLETA");
    dibujarCuadro(22, 9, 76, 10);
    gotoxy(28, 12);
    printf("Cedula del alumno: ");
    leerTexto(cedula, sizeof(cedula), 50, 12);

    posAlumno = buscarAlumnoPorCedulaMateria(cedula, materiaId);
    if (posAlumno < 0) {
        mostrarError("Alumno no encontrado.", 23);
        pausa();
        return;
    }

    if (!generarArchivoBoleta(materiaId, posAlumno, ruta)) {
        mostrarError("No se pudo generar el archivo de la boleta.", 23);
        pausa();
        return;
    }

    if (imprimirArchivoTexto(ruta)) {
        mostrarExito("Boleta enviada a imprimir con ShellAPI.", 23);
    } else {
        mostrarError("No se pudo enviar a imprimir. Revise impresora o app .txt.", 23);
    }
    gotoxy(8, 24);
    printf("Archivo generado: %s", ruta);
    pausa();
}

void verActaMateria(int materiaId, int modoAdmin) {
    int posMateria = buscarMateriaPorId(materiaId);
    int i, j;
    int totalEvaluaciones;
    int y = 15;
    int completo;
    float definitiva;
    char docente[120];

    if (posMateria < 0) {
        mostrarError("Materia no encontrada.", 23);
        pausa();
        return;
    }

    (void) modoAdmin;
    totalEvaluaciones = materias[posMateria].totalEvaluaciones;
    if (totalEvaluaciones <= 0 || totalEvaluaciones > MAX_EVALS) totalEvaluaciones = 3;

    nombreDocente(materias[posMateria].docenteId, docente);
    pantallaBase("ACTA DE CALIFICACIONES");
    dibujarCuadro(4, 8, 112, 18);
    gotoxy(8, 9);
    printf("Materia: %s  Codigo: %s  Periodo: %s", materias[posMateria].nombre, materias[posMateria].codigo, materias[posMateria].periodo);
    gotoxy(8, 10);
    printf("Usuario: %s", docente);
    gotoxy(8, 11);
    printf("Plan:");
    for (j = 0; j < totalEvaluaciones; j++) {
        gotoxy(14 + (j % 3) * 33, 11 + (j / 3));
        printf("E%d %.10s/%.8s %.0f%%",
               j + 1,
               materias[posMateria].evalNombres[j],
               materias[posMateria].evalTipos[j],
               materias[posMateria].ponderaciones[j]);
    }
    gotoxy(8, 14);
    printf("CEDULA       NOMBRES          APELLIDOS        ");
    for (j = 0; j < totalEvaluaciones; j++) {
        printf("E%-2d    ", j + 1);
    }
    printf("PROM.   MENSAJE");

    for (i = 0; i < totalAlumnos && y < 25; i++) {
        if (alumnos[i].materiaId == materiaId) {
            definitiva = calcularDefinitivaAlumno(&alumnos[i], &materias[posMateria], &completo);
            gotoxy(8, y);
            printf("%-12s %-16.16s %-16.16s",
                   alumnos[i].cedula,
                   alumnos[i].nombres,
                   alumnos[i].apellidos);
            for (j = 0; j < totalEvaluaciones; j++) {
                if (alumnos[i].tieneNotas[j]) printf(" %5.2f ", alumnos[i].notas[j]);
                else printf("  0.00 ");
            }
            (void) completo;
            printf(" %6.2f %s", definitiva, condicionAlumno(&alumnos[i], &materias[posMateria]));
            y++;
        }
    }

    if (y == 15) {
        mostrarError("No hay alumnos para esta acta.", 18);
    }

    gotoxy(8, 26);
    printf("Acta generada: %s", materias[posMateria].actaGenerada ? "Si" : "No");
    pausa();
}

int prepararCarpetaImpresiones(void) {
    if (CreateDirectoryA(CARPETA_IMPRESIONES, NULL)) return 1;
    return GetLastError() == ERROR_ALREADY_EXISTS;
}

int generarArchivoActa(int materiaId, char ruta[MAX_PATH]) {
    FILE *archivo;
    int posMateria = buscarMateriaPorId(materiaId);
    int totalEvaluaciones;
    int i, j;
    int completo;
    float definitiva;
    char usuario[120];

    if (posMateria < 0 || !prepararCarpetaImpresiones()) return 0;

    totalEvaluaciones = materias[posMateria].totalEvaluaciones;
    if (totalEvaluaciones <= 0 || totalEvaluaciones > MAX_EVALS) totalEvaluaciones = 3;
    nombreDocente(materias[posMateria].docenteId, usuario);

    snprintf(ruta, MAX_PATH, "%s\\acta_%s.txt", CARPETA_IMPRESIONES, materias[posMateria].codigo);
    archivo = fopen(ruta, "w");
    if (archivo == NULL) return 0;

    fprintf(archivo, "ACTA DE CALIFICACIONES\n");
    fprintf(archivo, "Materia: %s\n", materias[posMateria].nombre);
    fprintf(archivo, "Codigo: %s\n", materias[posMateria].codigo);
    fprintf(archivo, "Periodo: %s\n", materias[posMateria].periodo);
    fprintf(archivo, "Usuario: %s\n\n", usuario);
    fprintf(archivo, "Plan de estudio:\n");
    for (i = 0; i < totalEvaluaciones; i++) {
        fprintf(archivo, "E%d - %s | Tipo: %s | Ponderacion: %.2f%%\n",
                i + 1,
                materias[posMateria].evalNombres[i],
                materias[posMateria].evalTipos[i],
                materias[posMateria].ponderaciones[i]);
    }

    fprintf(archivo, "\n%-12s %-18s %-18s", "CEDULA", "NOMBRES", "APELLIDOS");
    for (i = 0; i < totalEvaluaciones; i++) {
        fprintf(archivo, " E%-2d   ", i + 1);
    }
    fprintf(archivo, " PROM.   MENSAJE\n");

    for (i = 0; i < totalAlumnos; i++) {
        if (alumnos[i].materiaId != materiaId) continue;
        definitiva = calcularDefinitivaAlumno(&alumnos[i], &materias[posMateria], &completo);
        fprintf(archivo, "%-12s %-18.18s %-18.18s",
                alumnos[i].cedula,
                alumnos[i].nombres,
                alumnos[i].apellidos);
        for (j = 0; j < totalEvaluaciones; j++) {
            fprintf(archivo, " %6.2f", alumnos[i].tieneNotas[j] ? alumnos[i].notas[j] : 0.0f);
        }
        (void) completo;
        fprintf(archivo, " %7.2f %s\n", definitiva, condicionAlumno(&alumnos[i], &materias[posMateria]));
    }

    fclose(archivo);
    return 1;
}

int generarArchivoBoleta(int materiaId, int posAlumno, char ruta[MAX_PATH]) {
    FILE *archivo;
    int posMateria = buscarMateriaPorId(materiaId);
    int totalEvaluaciones;
    int i;
    int completo;
    float definitiva;
    char usuario[120];

    if (posMateria < 0 || posAlumno < 0 || !prepararCarpetaImpresiones()) return 0;

    totalEvaluaciones = materias[posMateria].totalEvaluaciones;
    if (totalEvaluaciones <= 0 || totalEvaluaciones > MAX_EVALS) totalEvaluaciones = 3;
    nombreDocente(materias[posMateria].docenteId, usuario);
    definitiva = calcularDefinitivaAlumno(&alumnos[posAlumno], &materias[posMateria], &completo);

    snprintf(ruta, MAX_PATH, "%s\\boleta_%s_%s.txt",
             CARPETA_IMPRESIONES,
             materias[posMateria].codigo,
             alumnos[posAlumno].cedula);
    archivo = fopen(ruta, "w");
    if (archivo == NULL) return 0;

    fprintf(archivo, "BOLETA DE CALIFICACIONES\n");
    fprintf(archivo, "Materia: %s\n", materias[posMateria].nombre);
    fprintf(archivo, "Usuario: %s\n", usuario);
    fprintf(archivo, "Alumno: %s %s\n", alumnos[posAlumno].nombres, alumnos[posAlumno].apellidos);
    fprintf(archivo, "Cedula: %s\n\n", alumnos[posAlumno].cedula);

    for (i = 0; i < totalEvaluaciones; i++) {
        fprintf(archivo, "E%d - %-24s %-14s %.2f%%  Nota: %.2f\n",
                i + 1,
                materias[posMateria].evalNombres[i],
                materias[posMateria].evalTipos[i],
                materias[posMateria].ponderaciones[i],
                alumnos[posAlumno].tieneNotas[i] ? alumnos[posAlumno].notas[i] : 0.0f);
    }

    (void) completo;
    fprintf(archivo, "\nPromedio: %.2f / 100\n", definitiva);
    fprintf(archivo, "Mensaje: %s\n", condicionAlumno(&alumnos[posAlumno], &materias[posMateria]));

    fclose(archivo);
    return 1;
}

int imprimirArchivoTexto(const char *ruta) {
    char rutaCompleta[MAX_PATH];
    HINSTANCE resultado;

    if (GetFullPathNameA(ruta, MAX_PATH, rutaCompleta, NULL) == 0) {
        return 0;
    }

    resultado = ShellExecuteA(NULL, "print", rutaCompleta, NULL, NULL, SW_HIDE);
    return (INT_PTR) resultado > 32;
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
