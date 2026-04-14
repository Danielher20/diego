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
Administrador:
admin / admin123

Docente:
El usuario y la clave los crea el administrador desde su panel.
```

## Que hace

- Menu principal con flechas.
- Login para administrador/director.
- Login para docentes creados por el administrador.
- El administrador registra docentes y decide usuario/clave.
- El administrador registra materias.
- El administrador asigna docentes por materia.
- El administrador supervisa actas.
- El administrador ve reportes globales.
- Cada docente ve solo sus materias asignadas.
- Cada docente registra alumnos por materia.
- Cada docente lista sus alumnos.
- Cada docente configura el plan de evaluacion.
- Cada docente carga o corrige notas.
- Cada docente genera actas finales.
- Guardar datos en `siga_sencillo.dat`.

## Flujo recomendado

1. Entrar como administrador con `admin / admin123`.
2. Registrar uno o varios docentes.
3. Crear las materias.
4. Asignar cada materia a un docente.
5. Cerrar sesion.
6. Entrar como docente con el usuario y clave creados por el administrador.
7. Registrar alumnos, configurar evaluaciones, cargar notas y generar actas.

## Nota

Los archivos HTML (`index.html`, `estilos.css`, `app.js`) quedan como una version grafica extra, pero la version pedida ahora es la de terminal.
