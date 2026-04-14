$gcc = 'C:\msys64\ucrt64\bin\gcc.exe'

if (!(Test-Path $gcc)) {
    Write-Host "No encontre GCC en $gcc"
    Write-Host "Instala MSYS2 o cambia la ruta en este archivo."
    exit 1
}

& $gcc -std=c11 -Wall -Wextra -pedantic .\siga_sencillo.c -o .\siga_sencillo.exe

if ($LASTEXITCODE -eq 0) {
    Write-Host "Compilado: siga_sencillo.exe"
} else {
    Write-Host "Hubo errores al compilar."
}
