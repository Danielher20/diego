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
apellido, usuario y contrasena.
```

## Que hace

- Menu principal con flechas.
- Login para usuarios creados al inicio o desde el menu principal.
- Cada usuario puede crear y cambiar entre distintas secciones.
- Cada seccion maneja su propia materia, alumnos, plan de estudio y notas.
- Menu `Gestion de alumnos` para registrar, listar, editar y retirar alumnos.
- Menu `Gestion academico` para plan de estudio, notas, actas e impresion.
- Cada usuario registra alumnos con nombres, apellidos y cedula.
- La edicion de alumnos se hace desde un listado de seleccion; ahi mismo se pueden retirar.
- Los campos de edicion no pueden quedar vacios.
- Cada usuario configura, edita o borra el plan de estudio por seccion.
- Las ponderaciones del plan deben sumar 100%.
- Cada nota se carga de forma individual por evaluacion y alumno.
- Las notas van de 1 a 20 puntos.
- Cada usuario genera actas finales con promedio y condicion.
- Puede generar PDF imprimibles de actas y boletas en la carpeta `impresiones`.
- Guardar datos en `siga_sencillo.dat`.

## Flujo recomendado

1. Ejecutar el programa.
2. Crear el primer usuario cuando el sistema lo solicite.
3. Iniciar sesion con ese usuario y contrasena.
4. Crear una seccion.
5. Registrar alumnos en la seccion activa.
6. Configurar el plan de estudio de esa seccion.
7. Cargar notas por evaluacion.
8. Ver o imprimir el acta de calificaciones.

## Impresion

La libreria usada para generar PDF es **ReportLab** mediante el script
`pdf_report.py`. El envio a impresora se hace con **ShellAPI de Windows**,
incluida con `shellapi.h`, usando `ShellExecuteA`.

Instala ReportLab si Python no lo tiene:

```powershell
python -m pip install reportlab
```

## Nota

Los archivos HTML (`index.html`, `estilos.css`, `app.js`) quedan como una version grafica extra, pero la version pedida ahora es la de terminal.
