# SiGA - Sistema Integral de Gestión Académica

Implementación MVP del PRD en C + HTML/CSS/JS puro.

## Ejecutar

1. Instala GCC o MinGW.
2. Compila:

```bash
make clean && make
```

En Windows con MinGW, también puede funcionar:

```powershell
mingw32-make clean
mingw32-make
```

Si `mingw32-make` no aparece pero MSYS2 sí instaló GCC, compila directo desde PowerShell:

```powershell
cd C:\Users\Daniel\Desktop\diego
$gcc = 'C:\msys64\ucrt64\bin\gcc.exe'
$src = Get-ChildItem .\src\*.c | ForEach-Object { $_.FullName }
& $gcc -std=c11 -Wall -Wextra -pedantic -O2 $src -o .\siga.exe -lws2_32
```

3. Inicia el servidor:

```bash
./siga
```

4. Abre:

```text
http://localhost:8080
```

## Usuarios semilla

- Administrador: `admin` / `1234`
- Docente: `docente` / `1234`

Al primer arranque se crean los archivos binarios en `data/`, una materia demo, alumnos demo, plan de evaluación y calificaciones iniciales.

## API principal

- `POST /api/login`
- `POST /api/logout`
- `GET /api/admin/docentes`
- `POST /api/admin/docentes`
- `GET /api/admin/materias`
- `POST /api/admin/materias`
- `GET /api/docente/materias`
- `GET /api/docente/alumnos`
- `POST /api/docente/alumnos`
- `GET /api/docente/evaluaciones`
- `POST /api/docente/evaluaciones`
- `GET /api/docente/calificaciones`
- `POST /api/docente/calificaciones`
- `GET /api/docente/acta`
- `GET /api/docente/acta.csv`

Todas las rutas privadas requieren `token` como parámetro.

## Notas de implementación

- Persistencia en archivos binarios `.dat`.
- Sesiones en memoria con expiración de 8 horas.
- Contraseñas hasheadas con SHA-256.
- Carga de notas con validación 0-20.
- Corrección de notas con motivo obligatorio cuando se modifica una nota existente.
- Auditoría en `data/log_auditoria.dat`.
- Backup automático de los `.dat` al iniciar el servidor.
