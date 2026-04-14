#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

#define MAX_VOTANTES 1900
#define MAX_CANDIDATOS 25
#define USERNAME "admin"
#define PASSWORD "admin123"

// Constantes para dimensiones de pantalla
#define ANCHO_PANTALLA 120
#define ALTO_PANTALLA 30
#define ANCHO_CUADRO_PRINCIPAL 118
#define ALTO_CUADRO_PRINCIPAL 29

typedef struct {
    char nombre[50];
    char cedula[20];
    int haVotado;
} Votante;

typedef struct {
    int id;
    char nombre[50];
    int votos;
} Candidato;

Votante votantes[MAX_VOTANTES];
Candidato candidatos[MAX_CANDIDATOS];
int totalVotantes = 0;
int totalCandidatos = 0;
int votosEmitidos = 0;
int votacionIniciada = 0;
int usuarioLogueado = 0;
int esAdministrador = 0;

// Prototipos de funciones
void menuPrincipal(void);
void menuAdministrador(void);
void registrarCandidato(void);
void procesoVotacion(void);
void verResultados(void);
void guardarDatos(void);
void cargarDatos(void);
int validarCedula(const char *cedula);
int buscarVotantePorCedula(const char *cedula);
int generarNuevoIdCandidato(void);
int login(void);
void activarVotacion(void);
void mostrarBarraCarga(const char* mensaje);
void limpiarLinea(int y);
void dibujarCuadro(int x, int y, int ancho, int alto);
void cuadroPrincipal(void);
void pausa(void);
void ocultarCursor(void);
void activarCursor(void);
void capitalizar(char* texto);
void obtenerFecha(char* fecha_out);
int confirmacionSiNo(char *mensaje);
void color(int color);
int validarNombre(const char *nombre);
void mostrarMensajeError(const char* mensaje, int y);
void mostrarMensajeExito(const char* mensaje, int y);
void gotoxy(int x, int y);
void limpiarPantalla(void);
void centrarTexto(const char* texto, int y);
void leerCadena(char *texto, int tamano, int x, int y);
void leerNumeros(char *texto, int tamano, int x, int y);
void listarCandidatos(void);
void desactivarVotacion(void);
void establecerFondoAzul(void);
void dibujarMenuPrincipal(int opcion);
void dibujarMenuAdministrador(int opcion);
void mostrarMensajeEnCuadro(const char* mensaje, int x, int y, int ancho, int colorFondo);
void limpiarArea(int x, int y, int ancho, int alto);
void dibujarPantallaPrincipal(void);
void dibujarPantallaAdministrador(void);
void procesoVotacionBucle(void);
int registrarVotanteAutomatico(const char *cedula, const char *nombre);
void configurarConsola(void);

// FUNCIÓN PARA CONFIGURAR LA CONSOLA
void configurarConsola(void) {
    // Configurar tamaño de la consola
    char comando[50];
    snprintf(comando, sizeof(comando), "mode con: cols=%d lines=%d", ANCHO_PANTALLA, ALTO_PANTALLA);
    system(comando);
    
    // Establecer título de la ventana
    SetConsoleTitle("Sistema de Votacion Electronica");
    
    // Ocultar cursor inicialmente
    ocultarCursor();
}

// FUNCIÓN PARA ESTABLECER FONDO AZUL CLARO
void establecerFondoAzul() {
    system("color 97");
}

void color(int color_val) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color_val);
}

void ocultarCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

void activarCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = TRUE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void limpiarPantalla() {
    system("cls");
}

void limpiarArea(int x, int y, int ancho, int alto) {
    for (int i = 0; i < alto; i++) {
        gotoxy(x, y + i);
        for (int j = 0; j < ancho; j++) {
            printf(" ");
        }
    }
}

// FUNCIÓN: Dibujar pantalla principal (sin limpiar pantalla)
void dibujarPantallaPrincipal(void) {
    establecerFondoAzul();
    cuadroPrincipal();
    dibujarCuadro(11, 2, 99, 3);
    mostrarMensajeEnCuadro("**** SISTEMA VOTACION ELECTRONICA ****", 11, 3, 99, 159);
    
    // Fecha y estadísticas
    char fecha[20];
    obtenerFecha(fecha);
    gotoxy(85, 5);
    printf("Fecha: %s", fecha);
    gotoxy(85, 6);
    printf("Candidatos: %d/%d", totalCandidatos, MAX_CANDIDATOS);
    gotoxy(85, 7);
    printf("Votos emitidos: %d", votosEmitidos);
    
    dibujarCuadro(25, 9, 70, 10);
    mostrarMensajeEnCuadro("Use flechas y ENTER para seleccionar", 25, 19, 70, 159);
    mostrarMensajeEnCuadro("Presione ESC para retroceder", 25, 20, 70, 159);
}

// FUNCIÓN: Dibujar pantalla administrador (sin limpiar pantalla)
void dibujarPantallaAdministrador(void) {
    establecerFondoAzul();
    cuadroPrincipal();
    dibujarCuadro(11, 2, 99, 3);
    mostrarMensajeEnCuadro("**** PANEL ADMINISTRADOR ****", 11, 3, 99, 159);
    dibujarCuadro(25, 7, 70, 12);
    mostrarMensajeEnCuadro("Use flechas y ENTER para seleccionar", 25, 22, 70, 159);
    mostrarMensajeEnCuadro("Presione ESC para retroceder", 25, 23, 70, 159);
}

// FUNCIÓN: Cuadro principal - CORREGIDA
void cuadroPrincipal() {
    int x = 1, y = 0, ancho = ANCHO_CUADRO_PRINCIPAL, alto = ALTO_CUADRO_PRINCIPAL;
    
    // Dibujar bordes del cuadro principal
    for (int fila = 0; fila < alto; fila++) {
        gotoxy(x, y + fila);
        for (int col = 0; col < ancho; col++) {
            if (fila == 0 && col == 0) {
                color(159);
                printf("%c", 201); // Esquina superior izquierda
            }
            else if (fila == 0 && col == ancho - 1) {
                color(159);
                printf("%c", 187); // Esquina superior derecha
            }
            else if (fila == alto - 1 && col == 0) {
                color(159);
                printf("%c", 200); // Esquina inferior izquierda
            }
            else if (fila == alto - 1 && col == ancho - 1) {
                color(159);
                printf("%c", 188); // Esquina inferior derecha
            }
            else if (col == 0 || col == ancho - 1) {
                color(159);
                printf("%c", 186); // Borde vertical
            }
            else if (fila == 0 || fila == alto - 1) {
                color(159);
                printf("%c", 205); // Borde horizontal
            }
            else {
                color(159);
                printf(" "); // Espacio interior
            }
        }
    }
    color(159);
}

// FUNCIÓN: Dibujar cuadro normal - CORREGIDA
void dibujarCuadro(int x, int y, int ancho, int alto) {
    for (int fila = 0; fila < alto; fila++) {
        gotoxy(x, y + fila);
        for (int col = 0; col < ancho; col++) {
            if (fila == 0 && col == 0) {
                color(159);
                printf("%c", 218); // Esquina superior izquierda
            }
            else if (fila == 0 && col == ancho - 1) {
                color(159);
                printf("%c", 191); // Esquina superior derecha
            }
            else if (fila == alto - 1 && col == 0) {
                color(159);
                printf("%c", 192); // Esquina inferior izquierda
            }
            else if (fila == alto - 1 && col == ancho - 1) {
                color(159);
                printf("%c", 217); // Esquina inferior derecha
            }
            else if (col == 0 || col == ancho - 1) {
                color(159);
                printf("%c", 179); // Borde vertical
            }
            else if (fila == 0 || fila == alto - 1) {
                color(159);
                printf("%c", 196); // Borde horizontal
            }
            else {
                color(159);
                printf(" "); // Espacio interior
            }
        }
    }
    color(159);
}

void mostrarMensajeEnCuadro(const char* mensaje, int x, int y, int ancho, int colorFondo) {
    int longitud = strlen(mensaje);
    int xCentro = x + (ancho - longitud) / 2;
    if (xCentro < x + 1) xCentro = x + 1;
    
    gotoxy(xCentro, y);
    color(colorFondo);
    printf("%s", mensaje);
    color(159);
}

// FUNCIÓN: Dibujar menú principal (solo las opciones)
void dibujarMenuPrincipal(int opcion) {
    char* opciones[] = {
        "1. Panel Administrador",
        "2. Votar", 
        "3. Ver resultados",
        "4. Salir"
    };
    
    for(int i = 0; i < 4; i++) {
        gotoxy(27, 10 + i);
        if(i + 1 == opcion) {
            color(30); // Resaltado
        } else {
            color(159); // Normal
        }
        printf("%-35s", opciones[i]);
        color(159);
    }
    
    gotoxy(27, 16);
    printf("Estado votacion: %s", votacionIniciada ? "EN PROCESO" : "FINALIZADA");
    
    gotoxy(27, 17);
    printf("Votos emitidos: %d", votosEmitidos);
    
    // Mostrar mensaje informativo sobre resultados
    if(votacionIniciada) {
        gotoxy(27, 19);
        color(207);
        printf("Resultados disponibles solo al finalizar");
        color(159);
    } else if(votosEmitidos > 0) {
        gotoxy(27, 19);
        color(158);
        printf("Resultados disponibles - Votacion finalizada");
        color(159);
    }
}

// FUNCIÓN: Dibujar menú administrador (solo las opciones)
void dibujarMenuAdministrador(int opcion) {
    char* opciones[] = {
        "1. Registrar candidato",
        "2. Listar candidatos",
        "3. Activar votacion", 
        "4. Desactivar votacion",
        "5. Ver resultados",
        "6. Cerrar sesion"
    };
    
    for(int i = 0; i < 6; i++) {
        gotoxy(27, 8 + i);
        if(i + 1 == opcion) {
            color(31); // Resaltado
        } else {
            color(159); // Normal
        }
        printf("%-35s", opciones[i]);
        color(159);
    }
}

void pausa() {
    color(159);
    centrarTexto("Presione cualquier tecla para continuar...", 27);
    getch();
}

void mostrarMensajeError(const char* mensaje, int y) {
    color(207);
    centrarTexto(mensaje, y);
    color(159);
}

void mostrarMensajeExito(const char* mensaje, int y) {
    color(158);
    centrarTexto(mensaje, y);
    color(159);
}

void leerCadena(char *texto, int tamano, int x, int y) {
    int cont = 0;
    char c;
    
    gotoxy(x, y);
    printf("                                        ");
    gotoxy(x, y);
    activarCursor();
    
    while (1) {
        c = getch();
        if (c == 13) { // Enter
            texto[cont] = '\0';
            break;
        } else if (c == 8 && cont > 0) { // Backspace
            cont--;
            gotoxy(x + cont, y);
            printf(" ");
            gotoxy(x + cont, y);
        } else if (c == 27) { // ESC
            texto[0] = '\0';
            break;
        } else if (isalpha(c) && cont < tamano - 1) {
            texto[cont++] = c;
            printf("%c", c);
        } else if (c == ' ' && cont > 0 && cont < tamano - 1) {
            texto[cont++] = c;
            printf("%c", c);
        }
    }
    ocultarCursor();
}

void leerNumeros(char *texto, int tamano, int x, int y) {
    int cont = 0;
    char c;
    
    gotoxy(x, y);
    printf("                                        ");
    gotoxy(x, y);
    activarCursor();
    
    while (1) {
        c = getch();
        if (c == 13) { // Enter
            texto[cont] = '\0';
            break;
        } else if (c == 8 && cont > 0) { // Backspace
            cont--;
            gotoxy(x + cont, y);
            printf(" ");
            gotoxy(x + cont, y);
        } else if (c == 27) { // ESC
            texto[0] = '\0';
            break;
        } else if (isdigit(c) && cont < tamano - 1) {
            texto[cont++] = c;
            printf("%c", c);
        }
    }
    ocultarCursor();
}

void capitalizar(char* texto) {
    if (strlen(texto) > 0) {
        texto[0] = toupper(texto[0]);
        for (int i = 1; texto[i]; i++) {
            texto[i] = tolower(texto[i]);
        }
    }
}

void obtenerFecha(char* fecha_out) {
    time_t ahora = time(NULL);
    struct tm *tiempo = localtime(&ahora);
    strftime(fecha_out, 20, "%d/%m/%Y", tiempo);
}

int esperarTecla() {
    int tecla = getch();
    
    if (tecla == 0 || tecla == 224) {
        tecla = getch();
        switch(tecla) {
            case 72: return 1;  // Flecha arriba
            case 80: return 2;  // Flecha abajo
            default: return tecla;
        }
    }
    
    if (tecla == 27) return 27; // ESC
    
    return tecla;
}

int confirmacionSiNo(char *mensaje) {
    int opcionSeleccionada = 1;
    int tecla;
    
    limpiarPantalla();
    establecerFondoAzul();
    cuadroPrincipal();
    
    dibujarCuadro(30, 7, 60, 9);
    mostrarMensajeEnCuadro(mensaje, 30, 9, 60, 159);
    
    while (1) {        
        gotoxy(45, 11);
        if (opcionSeleccionada == 1) {
            color(31);
            printf("-> Si <-");
        } else {
            color(159);
            printf("   Si   ");
        }
        color(159);
        
        gotoxy(45, 13);
        if (opcionSeleccionada == 2) {
            color(31);
            printf("-> No <-");
        } else {
            color(159);
            printf("   No   ");
        }
        color(159);
        
        tecla = esperarTecla();
        
        if (tecla == 27) return -1; // ESC para cancelar
        
        if (tecla == 1 || tecla == 2) {
            opcionSeleccionada = (opcionSeleccionada == 1) ? 2 : 1;
        } else if (tecla == 13) {
            return opcionSeleccionada;
        }
    }
}

void mostrarBarraCarga(const char* mensaje) {
    limpiarPantalla();
    establecerFondoAzul();
    cuadroPrincipal();
    
    dibujarCuadro(25, 10, 70, 8);
    mostrarMensajeEnCuadro("**** SISTEMA DE VOTACION ELECTRONICA ****", 11, 3, 99, 159);
    mostrarMensajeEnCuadro(mensaje, 25, 12, 70, 159);
    
    int xBarra = 34;
    gotoxy(xBarra, 14);
    color(159);
    printf("[");
    for(int i = 0; i < 50; i++) {
        printf("%c", 177);
    }
    printf("]");
    
    gotoxy(xBarra, 14);
    printf("[");
    for(int i = 0; i < 50; i++) {
        color(31);
        printf("%c", 219);
        fflush(stdout);
        Sleep(30);
    }
    printf("]");
    color(159);
    
    Sleep(500);
}

void limpiarLinea(int y) {
    gotoxy(3, y);
    color(159);
    for(int i = 3; i < ANCHO_PANTALLA - 3; i++) {
        printf(" ");
    }
    color(159);
}

void centrarTexto(const char* texto, int y) {
    int longitud = strlen(texto);
    int x = (ANCHO_PANTALLA - longitud) / 2;
    if (x < 0) x = 0;
    gotoxy(x, y);
    printf("%s", texto);
}

int validarCedula(const char *cedula) {
    if(cedula == NULL) return 0;
    if(strlen(cedula) != 8) return 0;
    for(int i = 0; i < 8; i++) {
        if(!isdigit((unsigned char)cedula[i])) return 0;
    }
    return 1;
}

int validarNombre(const char *nombre) {
    if(nombre == NULL || strlen(nombre) == 0) return 0;
    if(strlen(nombre) < 2) return 0;
    
    for(int i = 0; nombre[i]; i++) {
        if(!isalpha((unsigned char)nombre[i]) && nombre[i] != ' ') {
            return 0;
        }
    }
    return 1;
}

int buscarVotantePorCedula(const char *cedula) {
    for(int i = 0; i < totalVotantes; i++) {
        if(strcmp(votantes[i].cedula, cedula) == 0) {
            return i;
        }
    }
    return -1;
}

int generarNuevoIdCandidato(void) {
    int maxId = 0;
    for(int i = 0; i < totalCandidatos; i++) {
        if(candidatos[i].id > maxId) maxId = candidatos[i].id;
    }
    return maxId + 1;
}

// FUNCIÓN: Registrar votante automáticamente
int registrarVotanteAutomatico(const char *cedula, const char *nombre) {
    if(totalVotantes >= MAX_VOTANTES) {
        return 0;
    }
    
    Votante nuevo;
    strncpy(nuevo.cedula, cedula, 19);
    nuevo.cedula[19] = '\0';
    
    if(nombre != NULL && strlen(nombre) > 0) {
        strncpy(nuevo.nombre, nombre, 49);
        nuevo.nombre[49] = '\0';
        capitalizar(nuevo.nombre);
    } else {
        // Si no se proporciona nombre, usar "Votante [Cédula]"
        snprintf(nuevo.nombre, 50, "Votante %s", cedula);
    }
    
    nuevo.haVotado = 0;
    votantes[totalVotantes++] = nuevo;
    return 1;
}

// FUNCIÓN: Login
int login(void) {
    char username[50];
    char password[50];
    int intentos = 3;
    
    while(intentos > 0) {
        limpiarPantalla();
        establecerFondoAzul();
        cuadroPrincipal();
        dibujarCuadro(11, 2, 99, 3);
        mostrarMensajeEnCuadro("**** L O G I N ****", 11, 3, 99, 159);

        dibujarCuadro(40, 10, 40, 7);
        
        // Dibujar etiquetas
        gotoxy(42, 12);
        color(159);
        printf("Usuario: ");
        gotoxy(42, 14);
        printf("Contrasena: ");
        
        // Limpiar áreas de entrada
        gotoxy(51, 12);
        printf("               ");
        gotoxy(53, 14);
        printf("               ");
        
        // Leer usuario
        gotoxy(51, 12);
        activarCursor();
        int i = 0;
        char c;
        while((c = getch()) != 13) {
            if(c == 8 && i > 0) {
                i--;
                gotoxy(51 + i, 12);
                printf(" ");
                gotoxy(51 + i, 12);
            } else if(c == 27) { // ESC para cancelar
                ocultarCursor();
                return 0;
            } else if(i < 14 && c != 13) {
                printf("%c", c);
                username[i++] = c;
            }
        }
        username[i] = '\0';
        
        // Leer contraseña
        gotoxy(53, 14);
        i = 0;
        while((c = getch()) != 13) {
            if(c == 8 && i > 0) {
                i--;
                gotoxy(53 + i, 14);
                printf(" ");
                gotoxy(53 + i, 14);
            } else if(c == 27) { // ESC para cancelar
                ocultarCursor();
                return 0;
            } else if(i < 14 && c != 13) {
                printf("*");
                password[i++] = c;
            }
        }
        password[i] = '\0';
        
        ocultarCursor();
        
        if(strcmp(username, USERNAME) == 0 && strcmp(password, PASSWORD) == 0) {
            usuarioLogueado = 1;
            esAdministrador = 1;
            mostrarBarraCarga("INICIANDO SESION ADMINISTRADOR");
            return 1;
        }
        
        intentos--;
        if (intentos > 0) {
            mostrarMensajeError("Credenciales incorrectas", 19);
            gotoxy(50, 20);
            printf("Intentos restantes: %d", intentos);
            Sleep(2000);
        }
    }
    
    mostrarMensajeError("Demasiados intentos fallidos. Acceso bloqueado.", 19);
    Sleep(2000);
    return 0;
}

// FUNCIÓN: Registrar candidato
void registrarCandidato(void) {
    if(votacionIniciada) {
        mostrarMensajeError("No se puede registrar durante votacion", 10);
        pausa();
        return;
    }
    
    if(totalCandidatos >= MAX_CANDIDATOS) {
        mostrarMensajeError("Capacidad maxima de candidatos alcanzada", 10);
        pausa();
        return;
    }
    
    Candidato nuevo;
    limpiarPantalla();
    establecerFondoAzul();
    cuadroPrincipal();
    dibujarCuadro(11, 2, 99, 3);
    mostrarMensajeEnCuadro("**** REGISTRO DE CANDIDATO ****", 11, 3, 99, 159);
    
    dibujarCuadro(25, 7, 70, 10);
    
    nuevo.id = generarNuevoIdCandidato();
    
    gotoxy(27, 9);
    printf("Nombre del candidato: ");
    
    int nombreValido = 0;
    while (!nombreValido) {
        leerCadena(nuevo.nombre, 30, 49, 9);
        
        if(strlen(nuevo.nombre) == 0) {
            return; // Usuario presionó ESC
        }
        
        if(strlen(nuevo.nombre) == 0) {
            mostrarMensajeError("El nombre no puede estar vacio", 16);
            Sleep(1500);
            limpiarLinea(16);
            continue;
        }
        
        if(!validarNombre(nuevo.nombre)) {
            mostrarMensajeError("Solo se permiten letras y espacios (min. 2 caracteres)", 16);
            Sleep(1500);
            limpiarLinea(16);
        } else {
            capitalizar(nuevo.nombre);
            nombreValido = 1;
        }
    }
    
    nuevo.votos = 0;
    candidatos[totalCandidatos++] = nuevo;
    
    mostrarBarraCarga("REGISTRANDO CANDIDATO");
    
    limpiarPantalla();
    establecerFondoAzul();
    cuadroPrincipal();
    dibujarCuadro(35, 10, 50, 5);
    mostrarMensajeEnCuadro("Candidato registrado exitosamente!", 35, 12, 50, 158);
    gotoxy(37, 13);
    printf("ID asignado: %d", nuevo.id);
    pausa();
}

// FUNCIÓN: Listar candidatos
void listarCandidatos(void) {
    limpiarPantalla();
    establecerFondoAzul();
    cuadroPrincipal();
    dibujarCuadro(11, 2, 99, 3);
    mostrarMensajeEnCuadro("**** LISTA DE CANDIDATOS ****", 11, 3, 99, 159);
    
    if(totalCandidatos == 0) {
        dibujarCuadro(35, 10, 50, 5);
        mostrarMensajeEnCuadro("No hay candidatos registrados", 35, 12, 50, 207);
        pausa();
        return;
    }
    
    int altoCuadro = 7 + (totalCandidatos > 15 ? 15 : totalCandidatos);
    if (altoCuadro > 20) altoCuadro = 20;
    dibujarCuadro(15, 7, 90, altoCuadro);
    
    gotoxy(20, 9);
    printf("ID   NOMBRE                      VOTOS");
    gotoxy(20, 10);
    printf("---- --------------------------- ------");
    
    for(int i = 0; i < totalCandidatos && i < 15; i++) {
        gotoxy(20, 11 + i);
        printf("%-4d %-28s %d", 
               candidatos[i].id, 
               candidatos[i].nombre,
               candidatos[i].votos);
    }
    
    if(totalCandidatos > 15) {
        gotoxy(20, 27);
        printf("... y %d candidatos mas", totalCandidatos - 15);
    }
    
    pausa();
}

// FUNCIÓN: Desactivar votación
void desactivarVotacion(void) {
    if(!votacionIniciada) {
        mostrarMensajeError("La votacion ya esta inactiva", 10);
        pausa();
        return;
    }
    
    limpiarPantalla();
    establecerFondoAzul();
    cuadroPrincipal();
    dibujarCuadro(11, 2, 99, 3);
    mostrarMensajeEnCuadro("**** DESACTIVAR VOTACION ****", 11, 3, 99, 159);
    
    dibujarCuadro(25, 7, 70, 10);
    
    mostrarMensajeEnCuadro("¿Desactivar proceso de votacion?", 25, 9, 70, 159);
    mostrarMensajeEnCuadro("Una vez desactivado, se podran:", 25, 10, 70, 159);
    mostrarMensajeEnCuadro("• Registrar nuevos candidatos", 25, 11, 70, 159);
    
    int confirmar = confirmacionSiNo("Confirmar desactivacion de votacion?");
    
    if(confirmar == 1) {
        votacionIniciada = 0;
        mostrarBarraCarga("DESACTIVANDO VOTACION");
        
        limpiarPantalla();
        establecerFondoAzul();
        cuadroPrincipal();
        dibujarCuadro(35, 10, 50, 5);
        mostrarMensajeEnCuadro("VOTACION DESACTIVADA!", 35, 12, 50, 158);
    } else if(confirmar == -1) {
        return; // Usuario presionó ESC
    } else {
        mostrarMensajeEnCuadro("Votacion no desactivada.", 25, 16, 70, 207);
    }
    pausa();
}

// FUNCIÓN: Proceso de votación en bucle para votantes
void procesoVotacionBucle(void) {
    if(!votacionIniciada) {
        mostrarMensajeError("La votacion no ha comenzado", 10);
        pausa();
        return;
    }
    
    if(totalCandidatos == 0) {
        mostrarMensajeError("No hay candidatos registrados", 10);
        pausa();
        return;
    }
    
    int continuar = 1;
    
    while(continuar && votacionIniciada) {
        limpiarPantalla();
        establecerFondoAzul();
        cuadroPrincipal();
        dibujarCuadro(11, 2, 99, 3);
        mostrarMensajeEnCuadro("**** PROCESO DE VOTACION ****", 11, 3, 99, 159);
        
        dibujarCuadro(25, 7, 70, 8);
        
        char cedula[20];
        gotoxy(27, 9);
        printf("Ingrese su cedula (8 digitos): ");
        
        int cedulaValida = 0;
        while (!cedulaValida) {
            leerNumeros(cedula, 9, 58, 9);
            
            if(strlen(cedula) == 0) {
                return; // Usuario presionó ESC
            }
            
            if(!validarCedula(cedula)) {
                mostrarMensajeError("La cedula debe tener 8 digitos", 12);
                Sleep(1500);
                limpiarLinea(12);
                continue;
            }
            cedulaValida = 1;
        }
        
        int indice = buscarVotantePorCedula(cedula);
        
        // Si el votante no existe, registrarlo automáticamente
        if(indice == -1) {
            if(!registrarVotanteAutomatico(cedula, NULL)) {
                mostrarMensajeError("Error al registrar votante", 12);
                pausa();
                continue;
            }
            indice = totalVotantes - 1;
            mostrarBarraCarga("REGISTRANDO NUEVO VOTANTE");
        }
        
        if(votantes[indice].haVotado) {
            mostrarMensajeError("Usted ya ha votado", 12);
            pausa();
            continue;
        }
        
        mostrarBarraCarga("VALIDANDO SU CEDULA");
        
        // Mostrar candidatos
        limpiarPantalla();
        establecerFondoAzul();
        cuadroPrincipal();
        dibujarCuadro(11, 2, 99, 3);
        mostrarMensajeEnCuadro("**** SELECCIONE CANDIDATO ****", 11, 3, 99, 159);
        
        int altoCuadro = 8 + totalCandidatos;
        if (altoCuadro > 20) altoCuadro = 20;
        dibujarCuadro(15, 7, 90, altoCuadro);
        
        // Ajustar nombre largo del votante
        char nombreAjustado[31];
        strncpy(nombreAjustado, votantes[indice].nombre, 30);
        nombreAjustado[30] = '\0';
        
        gotoxy(20, 9);
        printf("BIENVENIDO/A %s", nombreAjustado);
        
        gotoxy(20, 11);
        printf("CANDIDATOS DISPONIBLES:");
        
        // Mostrar candidatos
        for(int i = 0; i < totalCandidatos && i < 10; i++) {
            gotoxy(23, 13 + i);
            printf("%d. %s", candidatos[i].id, candidatos[i].nombre);
        }
        
        if (totalCandidatos > 10) {
            gotoxy(23, 13 + 10);
            printf("... y %d candidatos mas", totalCandidatos - 10);
        }
        
        int yPos = 13 + totalCandidatos + 2;
        if (yPos > 28) yPos = 26;
        gotoxy(20, yPos);
        printf("Ingrese el ID del candidato: ");
        
        char input[10];
        leerNumeros(input, 10, 48, yPos);
        
        if(strlen(input) == 0) {
            return; // Usuario presionó ESC
        }
        
        int voto = atoi(input);
        
        int candidatoValido = 0;
        for(int i = 0; i < totalCandidatos; i++) {
            if(candidatos[i].id == voto) {
                candidatos[i].votos++;
                votantes[indice].haVotado = 1;
                votosEmitidos++;
                candidatoValido = 1;
                
                mostrarBarraCarga("REGISTRANDO SU VOTO");
                
                limpiarPantalla();
                establecerFondoAzul();
                cuadroPrincipal();
                dibujarCuadro(35, 10, 50, 5);
                mostrarMensajeEnCuadro("VOTO REGISTRADO EXITOSAMENTE!", 35, 12, 50, 158);
                gotoxy(37, 13);
                printf("Candidato: %s", candidatos[i].nombre);
                break;
            }
        }
        
        if(!candidatoValido) {
            mostrarMensajeError("Candidato no valido", 12);
        }
        
        // Preguntar si continuar con otro votante (solo para administrador)
        if(esAdministrador) {
            int confirmar = confirmacionSiNo("¿Continuar con otro votante?");
            if(confirmar == 2 || confirmar == -1) { // No o ESC
                continuar = 0;
            }
        } else {
            // Para votantes normales, solo una votación
            continuar = 0;
        }
        
        pausa();
    }
}

// FUNCIÓN: Proceso de votación simple (para usuarios normales)
void procesoVotacion(void) {
    procesoVotacionBucle();
}

// FUNCIÓN: Activar votación
void activarVotacion(void) {
    if(totalCandidatos == 0) {
        mostrarMensajeError("No hay candidatos registrados", 10);
        pausa();
        return;
    }
    
    limpiarPantalla();
    establecerFondoAzul();
    cuadroPrincipal();
    dibujarCuadro(11, 2, 99, 3);
    mostrarMensajeEnCuadro("**** ACTIVAR VOTACION ****", 11, 3, 99, 159);
    
    dibujarCuadro(25, 7, 70, 10);
    
    mostrarMensajeEnCuadro("¿Iniciar proceso de votacion?", 25, 9, 70, 159);
    mostrarMensajeEnCuadro("Una vez iniciado:", 25, 10, 70, 159);
    mostrarMensajeEnCuadro("• Los votantes podran votar ingresando su cedula", 25, 11, 70, 159);
    mostrarMensajeEnCuadro("• Se registraran automaticamente como nuevos votantes", 25, 12, 70, 159);
    
    int confirmar = confirmacionSiNo("Confirmar activacion de votacion?");
    
    if(confirmar == 1) {
        votacionIniciada = 1;
        mostrarBarraCarga("INICIANDO VOTACION");
        
        limpiarPantalla();
        establecerFondoAzul();
        cuadroPrincipal();
        dibujarCuadro(35, 10, 50, 5);
        mostrarMensajeEnCuadro("VOTACION INICIADA!", 35, 12, 50, 158);
    } else if(confirmar == -1) {
        return; // Usuario presionó ESC
    } else {
        mostrarMensajeEnCuadro("Votacion no iniciada.", 25, 16, 70, 207);
    }
    pausa();
}

// FUNCIÓN: Ver resultados
void verResultados(void) {
    if(votacionIniciada) {
        mostrarMensajeError("Los resultados solo estan disponibles cuando la votacion ha finalizado", 10);
        pausa();
        return;
    }
    
    if(votosEmitidos == 0) {
        mostrarMensajeError("No se han emitido votos", 10);
        pausa();
        return;
    }
    
    if(totalCandidatos == 0) {
        mostrarMensajeError("No hay candidatos registrados", 10);
        pausa();
        return;
    }
    
    limpiarPantalla();
    establecerFondoAzul();
    cuadroPrincipal();
    dibujarCuadro(11, 2, 99, 3);
    mostrarMensajeEnCuadro("**** RESULTADOS FINALES DE VOTACION ****", 11, 3, 99, 159);
    
    int altoCuadro = 12 + totalCandidatos;
    if (altoCuadro > 20) altoCuadro = 20;
    dibujarCuadro(15, 7, 90, altoCuadro);
    
    gotoxy(20, 9);
    printf("RESULTADOS FINALES");
    gotoxy(20, 10);
    printf("==================");
    
    gotoxy(20, 12);
    printf("Total de votos emitidos: %d", votosEmitidos);
    gotoxy(20, 13);
    printf("Total de votantes registrados: %d", totalVotantes);
    
    // Calcular porcentaje de participación
    float participacion = 0.0;
    if (totalVotantes > 0) {
        participacion = (float)votosEmitidos / totalVotantes * 100;
    }
    gotoxy(20, 14);
    printf("Participacion: %.1f%%", participacion);
    
    // Ordenar candidatos por votos
    Candidato tempCandidatos[MAX_CANDIDATOS];
    for(int i = 0; i < totalCandidatos; i++) {
        tempCandidatos[i] = candidatos[i];
    }
    
    for(int i = 0; i < totalCandidatos - 1; i++) {
        for(int j = 0; j < totalCandidatos - i - 1; j++) {
            if(tempCandidatos[j].votos < tempCandidatos[j + 1].votos) {
                Candidato temp = tempCandidatos[j];
                tempCandidatos[j] = tempCandidatos[j + 1];
                tempCandidatos[j + 1] = temp;
            }
        }
    }
    
    gotoxy(20, 16);
    printf("CANDIDATO                  VOTOS   PORCENTAJE");
    gotoxy(20, 17);
    printf("----------------------- ------- ----------");
    
    for(int i = 0; i < totalCandidatos && i < 10; i++) {
        float porcentaje = (float)tempCandidatos[i].votos / votosEmitidos * 100;
        gotoxy(20, 18 + i);
        if (i == 0 && tempCandidatos[i].votos > 0) {
            color(31); // Color rojo para el ganador
        }
        char nombreAjustado[26];
        strncpy(nombreAjustado, tempCandidatos[i].nombre, 25);
        nombreAjustado[25] = '\0';
        printf("%-25s %-7d %-10.2f%%", nombreAjustado, tempCandidatos[i].votos, porcentaje);
        color(159);
    }
    
    // Mostrar ganador
    if(totalCandidatos > 0 && tempCandidatos[0].votos > 0) {
        int posY = 19 + totalCandidatos;
        if (posY > 28) posY = 27;
        
        gotoxy(20, posY);
        color(31);
        printf("========================================");
        gotoxy(20, posY + 1);
        printf("GANADOR: %s", tempCandidatos[0].nombre);
        gotoxy(20, posY + 2);
        printf("Con %d votos (%.2f%%)", tempCandidatos[0].votos, 
              (float)tempCandidatos[0].votos / votosEmitidos * 100);
        color(159);
    }
    
    pausa();
}

void guardarDatos(void) {
    FILE *archivo;
    
    archivo = fopen("votantes.dat", "wb");
    if(archivo != NULL) {
        fwrite(&totalVotantes, sizeof(int), 1, archivo);
        fwrite(votantes, sizeof(Votante), totalVotantes, archivo);
        fclose(archivo);
    }
    
    archivo = fopen("candidatos.dat", "wb");
    if(archivo != NULL) {
        fwrite(&totalCandidatos, sizeof(int), 1, archivo);
        fwrite(candidatos, sizeof(Candidato), totalCandidatos, archivo);
        fclose(archivo);
    }
    
    archivo = fopen("sistema.dat", "wb");
    if(archivo != NULL) {
        fwrite(&votacionIniciada, sizeof(int), 1, archivo);
        fwrite(&votosEmitidos, sizeof(int), 1, archivo);
        fclose(archivo);
    }
}

void cargarDatos(void) {
    FILE *archivo;
    
    archivo = fopen("votantes.dat", "rb");
    if(archivo != NULL) {
        fread(&totalVotantes, sizeof(int), 1, archivo);
        if(totalVotantes > MAX_VOTANTES) totalVotantes = MAX_VOTANTES;
        fread(votantes, sizeof(Votante), totalVotantes, archivo);
        fclose(archivo);
    }
    
    archivo = fopen("candidatos.dat", "rb");
    if(archivo != NULL) {
        fread(&totalCandidatos, sizeof(int), 1, archivo);
        if(totalCandidatos > MAX_CANDIDATOS) totalCandidatos = MAX_CANDIDATOS;
        fread(candidatos, sizeof(Candidato), totalCandidatos, archivo);
        fclose(archivo);
    }
    
    archivo = fopen("sistema.dat", "rb");
    if(archivo != NULL) {
        fread(&votacionIniciada, sizeof(int), 1, archivo);
        fread(&votosEmitidos, sizeof(int), 1, archivo);
        fclose(archivo);
    }
}

void menuAdministrador(void) {
    int opcion = 1;
    int tecla;
    int salir = 0;
    
    // Dibujar pantalla completa solo una vez
    limpiarPantalla();
    dibujarPantallaAdministrador();
    dibujarMenuAdministrador(opcion);
    
    while(!salir && usuarioLogueado) {
        tecla = esperarTecla();
        
        if (tecla == 27) { // ESC para retroceder
            int confirma = confirmacionSiNo("¿Cerrar sesion de administrador?");
            if (confirma == 1) {
                usuarioLogueado = 0;
                esAdministrador = 0;
                mostrarBarraCarga("CERRANDO SESION");
                salir = 1;
            } else {
                // Redibujar pantalla después de la confirmación
                dibujarPantallaAdministrador();
                dibujarMenuAdministrador(opcion);
            }
            continue;
        }
        
        if (tecla == 1) {
            opcion--;
            if (opcion < 1) opcion = 6;
            dibujarMenuAdministrador(opcion);
        } else if (tecla == 2) {
            opcion++;
            if (opcion > 6) opcion = 1;
            dibujarMenuAdministrador(opcion);
        } else if (tecla == 13) {
            switch(opcion) {
                case 1: 
                    registrarCandidato();
                    dibujarPantallaAdministrador();
                    dibujarMenuAdministrador(opcion);
                    break;
                case 2: 
                    listarCandidatos(); 
                    dibujarPantallaAdministrador();
                    dibujarMenuAdministrador(opcion);
                    break;
                case 3: 
                    activarVotacion(); 
                    dibujarPantallaAdministrador();
                    dibujarMenuAdministrador(opcion);
                    break;
                case 4: 
                    desactivarVotacion(); 
                    dibujarPantallaAdministrador();
                    dibujarMenuAdministrador(opcion);
                    break;
                case 5: 
                    verResultados(); 
                    dibujarPantallaAdministrador();
                    dibujarMenuAdministrador(opcion);
                    break;
                case 6: 
                    usuarioLogueado = 0;
                    esAdministrador = 0;
                    mostrarBarraCarga("CERRANDO SESION");
                    salir = 1;
                    break;
            }
        }
    }
}

void menuPrincipal(void) {
    int opcion = 1;
    int tecla;
    int salir = 0;
    
    configurarConsola();
    establecerFondoAzul();
    
    // Dibujar pantalla completa solo una vez
    limpiarPantalla();
    dibujarPantallaPrincipal();
    dibujarMenuPrincipal(opcion);
    
    while(!salir) {
        tecla = esperarTecla();
        
        if (tecla == 27) { // ESC para salir
            int confirma = confirmacionSiNo("¿Salir del sistema?");
            if (confirma == 1) {
                guardarDatos();
                salir = 1;
            } else {
                // Redibujar pantalla después de la confirmación
                dibujarPantallaPrincipal();
                dibujarMenuPrincipal(opcion);
            }
            continue;
        }
        
        if (tecla == 1) {
            opcion--;
            if (opcion < 1) opcion = 4;
            dibujarMenuPrincipal(opcion);
        } else if (tecla == 2) {
            opcion++;
            if (opcion > 4) opcion = 1;
            dibujarMenuPrincipal(opcion);
        } else if (tecla == 13) {
            switch(opcion) {
                case 1:
                    if(login()) {
                        menuAdministrador();
                        // Redibujar después de volver
                        dibujarPantallaPrincipal();
                        dibujarMenuPrincipal(opcion);
                    } else {
                        // Redibujar si el login falla
                        dibujarPantallaPrincipal();
                        dibujarMenuPrincipal(opcion);
                    }
                    break;
                case 2:
                    if(!votacionIniciada) {
                        mostrarMensajeError("La votacion no esta activa", 10);
                        pausa();
                        dibujarPantallaPrincipal();
                        dibujarMenuPrincipal(opcion);
                    } else {
                        procesoVotacion();
                        dibujarPantallaPrincipal();
                        dibujarMenuPrincipal(opcion);
                    }
                    break;
                case 3:
                    verResultados();
                    dibujarPantallaPrincipal();
                    dibujarMenuPrincipal(opcion);
                    break;
                case 4:
                    guardarDatos();
                    mostrarBarraCarga("GUARDANDO Y CERRANDO SISTEMA");
                    salir = 1;
                    break;
            }
        }
    }
}

int main(void) {
    configurarConsola();
    establecerFondoAzul();
    
    ocultarCursor();
    mostrarBarraCarga("CARGANDO SISTEMA DE VOTACION");
    cargarDatos();
    menuPrincipal();
    
    activarCursor();
    limpiarPantalla();
    establecerFondoAzul();
    centrarTexto("Sistema de votacion cerrado. ¡Hasta pronto!", 15);
    return 0;
}