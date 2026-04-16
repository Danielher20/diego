# SiGA Sencillo Terminal

Version sencilla del proyecto SiGA hecha en consola, con estilo parecido al archivo `2do centro votacion.c`.

Esta version usa:

- `windows.h`
- `conio.h`
- `gotoxy`
- colores de consola
- cuadros con caracteres ASCII
- menus con flechas
- ENTER para seleccionar
- ESC para retroceder

## Archivo principal

```text
siga_sencillo.c
```

## Compilar

Desde PowerShell:

```powershell
cd C:\Users\Daniel\Desktop\diego\sencillo
& 'C:\msys64\ucrt64\bin\gcc.exe' -std=c11 -Wall -Wextra -pedantic siga_sencillo.c -o siga_sencillo.exe
```

O usa:

```powershell
.\compilar.ps1
```

## Ejecutar

```powershell
.\siga_sencillo.exe
```

## Usuarios

```text
Usuario:
Al iniciar por primera vez, el sistema pide crear un usuario con nombre,
apellido, usuario, contrasena y materia a impartir.
```

## Que hace

- Menu principal con flechas.
- Login para usuarios creados al inicio o desde el menu principal.
- Cada usuario tiene una materia a impartir.
- Cada usuario registra alumnos con nombres, apellidos y cedula.
- Cada usuario puede editar alumnos o marcarlos como retirados.
- Cada usuario configura el plan de estudio con nombre, tipo y ponderacion.
- Las ponderaciones del plan deben sumar 100%.
- Cada usuario carga o corrige notas.
- Cada usuario genera actas finales con promedio y condicion.
- Guardar datos en `siga_sencillo.dat`.

## Flujo recomendado

1. Ejecutar el programa.
2. Crear el primer usuario cuando el sistema lo solicite.
3. Iniciar sesion con ese usuario y contrasena.
4. Registrar alumnos.
5. Configurar el plan de estudio.
6. Cargar notas.
7. Ver el acta de calificaciones.

## Nota

Los archivos HTML (`index.html`, `estilos.css`, `app.js`) quedan como una version grafica extra, pero la version pedida ahora es la de terminal.
