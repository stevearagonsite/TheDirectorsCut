# Comandos en Windows

Preferí `Tools/scripts/tdc.ps1`, que los envuelve. Esta página es la referencia cruda por si el script falla.
Las rutas de cada máquina van en `Tools/scripts/tdc.local.ps1` (ignorado por git; copiar de `tdc.local.example.ps1`).

| Variable | Ejemplo |
|---|---|
| `$UE` | `C:\Program Files\Epic Games\UE_5.6` |
| `$Blender` | `C:\Program Files\Blender Foundation\Blender 4.5\blender.exe` |
| `$Repo` | `D:\Games\TheDirectorsCut` |
| `$Proj` | `$Repo\Game\TheDirectorsCut.uproject` |

## Unreal

```powershell
# Generar archivos de proyecto (Visual Studio)
& "$UE\Engine\Build\BatchFiles\Build.bat" -projectfiles -project="$Proj" -game -progress

# Compilar el editor (lo que hay que hacer después de cada cambio de C++)
& "$UE\Engine\Build\BatchFiles\Build.bat" TheDirectorsCutEditor Win64 Development -Project="$Proj" -WaitMutex

# Tests de automatización (prefijo TDC.)
& "$UE\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "$Proj" -ExecCmds="Automation RunTests TDC.;Quit" `
  -unattended -nopause -nullrhi -nosplash -log -ReportExportPath="$Repo\Game\Saved\TestReport"

# Script Python dentro del editor (requiere plugin PythonScriptPlugin, ya habilitado en el .uproject)
& "$UE\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "$Proj" -run=pythonscript -script="$Repo\Tools\unreal\<script>.py"

# Abrir un mapa en modo juego y sacar captura
& "$UE\Engine\Binaries\Win64\UnrealEditor.exe" "$Proj" /Game/Maps/L_Slice -game -windowed -ResX=1280 -ResY=720 `
  -ExecCmds="HighResShot 1280x720" -log

# Empaquetar (más adelante)
& "$UE\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun -project="$Proj" -platform=Win64 -clientconfig=Development `
  -build -cook -stage -pak -archive -archivedirectory="$Repo\Game\Saved\Package"
```

Logs: `Game\Saved\Logs\TheDirectorsCut.log`. Capturas de HighResShot: `Game\Saved\Screenshots\WindowsEditor\`.

## Blender

```powershell
# Ejecutar un script sin interfaz sobre un .blend
& $Blender -b "$Repo\Art\Source\<asset>.blend" --python "$Repo\Tools\blender\export_fbx.py" -- --out "$Repo\Art\Export\<asset>.fbx"

# Render de previsualización
& $Blender -b "$Repo\Art\Source\<asset>.blend" -o "$Repo\docs\evidence\T-XXXX\preview_" -F PNG -f 1
```

## Git

```powershell
git config core.hooksPath .githooks     # una vez por clon: activa gitleaks en pre-commit
git lfs lock Game/Content/Maps/L_Slice.umap
git lfs unlock Game/Content/Maps/L_Slice.umap
git lfs locks                            # quién tiene qué
```
