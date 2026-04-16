# SiGA Sencillo Terminal

Version sencilla del proyecto SiGA hecha en consola, con estilo parecido al archivo `2do centro votacion.c`.

Esta version usa:

- `windows.h`
- `shellapi.h` para imprimir actas y boletas con ShellAPI
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
& 'C:\msys64\ucrt64\bin\gcc.exe' -std=c11 -Wall -Wextra -pedantic siga_sencillo.c -o siga_sencillo.exe -lshell32
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
- Menu `Gestion de alumnos` para registrar, listar, editar y retirar alumnos.
- Menu `Gestion academico` para plan de estudio, notas, actas e impresion.
- Cada usuario registra alumnos con nombres, apellidos y cedula.
- Cada usuario puede editar alumnos o marcarlos como retirados.
- Cada usuario configura el plan de estudio con nombre, tipo y ponderacion.
- Las ponderaciones del plan deben sumar 100%.
- Cada nota se carga de forma individual por evaluacion y alumno.
- Las notas van de 0 a 100 puntos.
- Cada usuario genera actas finales con promedio y condicion.
- Puede generar archivos imprimibles de actas y boletas en la carpeta `impresiones`.
- Guardar datos en `siga_sencillo.dat`.

## Flujo recomendado

1. Ejecutar el programa.
2. Crear el primer usuario cuando el sistema lo solicite.
3. Iniciar sesion con ese usuario y contrasena.
4. Registrar alumnos.
5. Configurar el plan de estudio.
6. Cargar notas por evaluacion.
7. Ver o imprimir el acta de calificaciones.

## Impresion

La libreria usada para imprimir es **ShellAPI de Windows**, incluida con `shellapi.h`.
El programa genera archivos `.txt` en `impresiones` y los envia a imprimir con
`ShellExecuteA`.

## Nota

Los archivos HTML (`index.html`, `estilos.css`, `app.js`) quedan como una version grafica extra, pero la version pedida ahora es la de terminal.
