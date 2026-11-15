<#
.SYNOPSIS
  Comandos del estudio Windows para The Director's Cut.

.EXAMPLE
  .\Tools\scripts\tdc.ps1 doctor
  .\Tools\scripts\tdc.ps1 projectfiles
  .\Tools\scripts\tdc.ps1 build
  .\Tools\scripts\tdc.ps1 test                 # tests TDC.*
  .\Tools\scripts\tdc.ps1 test -Filter TDC.Director
  .\Tools\scripts\tdc.ps1 editor
  .\Tools\scripts\tdc.ps1 python -Script Tools\unreal\import_assets.py
  .\Tools\scripts\tdc.ps1 blender -Blend Art\Source\Spotlight.blend -Script Tools\blender\export_fbx.py -- --out Art\Export\Spotlight.fbx
  .\Tools\scripts\tdc.ps1 shot -Map /Game/Maps/L_Slice -Task T-0002 -Name sala
  .\Tools\scripts\tdc.ps1 play                  # abre el juego en ventana (L_Slice) y no espera
  .\Tools\scripts\tdc.ps1 stop                  # cierra el editor/juego de ESTE proyecto y su Live Coding (antes de build)
  .\Tools\scripts\tdc.ps1 python -Script Tools\unreal\build_slice.py -InEditor   # si el commandlet no alcanza
  .\Tools\scripts\tdc.ps1 package                 # build Win64 Development + zip para la jam
  .\Tools\scripts\tdc.ps1 package -Config Shipping -Zip Equipo_Juego
  .\Tools\scripts\tdc.ps1 smoke -Task T-0006 -Name 01-package     # prueba el zip: arranca el exe, captura y log
  .\Tools\scripts\tdc.ps1 smoke -Task T-0006 -Name 02-final -Keep # igual, pero deja el juego abierto
#>
param(
  [Parameter(Position = 0, Mandatory = $true)]
  [ValidateSet('doctor', 'projectfiles', 'build', 'test', 'editor', 'python', 'blender', 'shot', 'play', 'stop', 'package', 'smoke')]
  [string]$Command,
  [string]$Filter = 'TDC.',
  [string]$Script,
  [string]$Blend,
  [string]$Map = '/Game/Maps/L_Slice',
  [string]$Task,
  [string]$Name = 'shot',
  [switch]$InEditor,
  [int]$Wait = 40,
  [switch]$Keep,
  [ValidateSet('Development', 'Shipping')]
  [string]$Config = 'Development',
  [string]$Zip = 'stevearagonsite_TheDirectorsCut',
  [Parameter(ValueFromRemainingArguments = $true)]
  [string[]]$Rest
)

$ErrorActionPreference = 'Stop'
$Repo = (Resolve-Path "$PSScriptRoot\..\..").Path
$Proj = Join-Path $Repo 'Game\TheDirectorsCut.uproject'

$local = Join-Path $PSScriptRoot 'tdc.local.ps1'
if (Test-Path $local) { . $local }
if (-not $UE)      { $UE = $env:TDC_UE }
if (-not $Blender) { $Blender = $env:TDC_BLENDER }

function Need-UE {
  if (-not $UE -or -not (Test-Path "$UE\Engine\Binaries\Win64\UnrealEditor.exe")) {
    throw "No encuentro Unreal. Definí `$UE en Tools\scripts\tdc.local.ps1 (ver tdc.local.example.ps1)."
  }
}
function Need-Blender {
  if (-not $Blender -or -not (Test-Path $Blender)) {
    throw "No encuentro Blender. Definí `$Blender en Tools\scripts\tdc.local.ps1."
  }
}
function Redact([string]$text) {
  return $text -replace [regex]::Escape($env:USERPROFILE), '%USERPROFILE%' -replace [regex]::Escape($Repo), '<repo>'
}
function Check-Exit([string]$what) {
  if ($LASTEXITCODE -ne 0) { throw "$what terminó con código $LASTEXITCODE" }
}

$UEBuild  = "$UE\Engine\Build\BatchFiles\Build.bat"
$UEEditor = "$UE\Engine\Binaries\Win64\UnrealEditor.exe"
$UECmd    = "$UE\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$UEUAT    = "$UE\Engine\Build\BatchFiles\RunUAT.bat"

switch ($Command) {
  'doctor' {
    "repo:     $Repo"
    "uproject: $(Test-Path $Proj)"
    "UE:       $UE  -> $(if ($UE) { Test-Path $UEEditor } else { $false })"
    "Blender:  $Blender -> $(if ($Blender) { Test-Path $Blender } else { $false })"
    foreach ($t in 'git', 'git-lfs', 'gitleaks', 'gh') {
      $c = Get-Command $t -ErrorAction SilentlyContinue
      "${t}: $(if ($c) { $c.Source } else { 'NO INSTALADO' })"
    }
    "hooksPath: $(git -C $Repo config core.hooksPath)"
  }
  'projectfiles' {
    Need-UE
    & $UEBuild -projectfiles -project="$Proj" -game -progress; Check-Exit 'projectfiles'
  }
  'build' {
    Need-UE
    $log = Join-Path $Repo 'Game\Saved\Logs\tdc-build.log'
    New-Item -ItemType Directory -Force (Split-Path $log) | Out-Null
    & $UEBuild TheDirectorsCutEditor Win64 Development -Project="$Proj" -WaitMutex 2>&1 | Tee-Object -FilePath $log
    $code = $LASTEXITCODE
    "`n--- errores/warnings (extracto) ---"
    Redact ((Select-String -Path $log -Pattern 'error|warning C' | Select-Object -First 40 | ForEach-Object Line) -join "`n")
    if ($code -ne 0) { throw "build falló (código $code). Log: Game\Saved\Logs\tdc-build.log" }
  }
  'test' {
    Need-UE
    $report = Join-Path $Repo 'Game\Saved\TestReport'
    & $UECmd "$Proj" -ExecCmds="Automation RunTests $Filter;Quit" -unattended -nopause -nullrhi -nosplash -log -ReportExportPath="$report"
    $code = $LASTEXITCODE
    $json = Join-Path $report 'index.json'
    if (Test-Path $json) {
      $r = Get-Content $json -Raw | ConvertFrom-Json
      "tests: ok=$($r.succeeded) fallidos=$($r.failed) warnings=$($r.succeededWithWarnings)"
      $r.tests | Where-Object { $_.state -ne 'Success' } | ForEach-Object {
        "FALLA $($_.fullTestPath)"; $_.entries | ForEach-Object { "   " + (Redact $_.event.message) }
      }
    }
    if ($code -ne 0) { throw "tests terminaron con código $code" }
  }
  'editor' {
    Need-UE
    Start-Process $UEEditor -ArgumentList "`"$Proj`""
  }
  'python' {
    Need-UE
    if (-not $Script) { throw 'Falta -Script' }
    $path = Join-Path $Repo $Script
    if ($InEditor) {
      # Editor completo sin ventana: el script tiene que cerrar el editor al terminar (build_slice.py lo hace).
      & $UECmd "$Proj" -ExecutePythonScript="$path" -unattended -nosplash -nopause 2>&1 | Tee-Object -Variable out
    } else {
      & $UECmd "$Proj" -run=pythonscript -script="$path" -unattended -nosplash 2>&1 | Tee-Object -Variable out
    }
    Check-Exit 'python'
    # Los scripts de Tools/unreal loguean "[TDC] ERROR" y el editor puede salir con 0 igual: cortar acá.
    if ($out -match '\[TDC\] ERROR') { throw 'el script reportó [TDC] ERROR (ver arriba)' }
  }
  'blender' {
    Need-Blender
    if (-not $Blend -or -not $Script) { throw 'Faltan -Blend y/o -Script' }
    & $Blender -b (Join-Path $Repo $Blend) --python (Join-Path $Repo $Script) -- @Rest; Check-Exit 'blender'
  }
  'shot' {
    Need-UE
    if (-not $Task) { throw 'Falta -Task (ej. T-0002)' }
    $shots = Join-Path $Repo 'Game\Saved\Screenshots\WindowsEditor'
    $before = Get-Date
    $p = Start-Process $UEEditor -PassThru -ArgumentList "`"$Proj`" $Map -game -windowed -ResX=1280 -ResY=720 -ExecCmds=`"HighResShot 1280x720`""
    Start-Sleep -Seconds 25
    Stop-Process -Id $p.Id -ErrorAction SilentlyContinue
    $png = Get-ChildItem $shots -Filter *.png -ErrorAction SilentlyContinue | Where-Object LastWriteTime -gt $before | Sort-Object LastWriteTime | Select-Object -Last 1
    if (-not $png) { throw 'No se generó captura (aumentar la espera o revisar el log)' }
    $dest = Join-Path $Repo "docs\evidence\$Task"
    New-Item -ItemType Directory -Force $dest | Out-Null
    Copy-Item $png.FullName (Join-Path $dest "$Name.png")
    "captura: docs/evidence/$Task/$Name.png"
  }
  'stop' {
    # Solo procesos de este proyecto (la línea de comandos contiene el .uproject) y sus LiveCodingConsole hijos.
    $procs = Get-CimInstance Win32_Process -Filter "Name='UnrealEditor.exe' OR Name='UnrealEditor-Cmd.exe'" |
      Where-Object { $_.CommandLine -and $_.CommandLine -like "*$Proj*" }
    $ids = @($procs | ForEach-Object { $_.ProcessId })
    $lc = Get-CimInstance Win32_Process -Filter "Name='LiveCodingConsole.exe'" | Where-Object { $ids -contains $_.ParentProcessId }
    foreach ($p in @($lc) + @($procs)) { if ($p) { Stop-Process -Id $p.ProcessId -Force -ErrorAction SilentlyContinue; "cerrado: $($p.Name) (pid $($p.ProcessId))" } }
    if (-not $ids.Count) { 'no había editor/juego de este proyecto abierto' }
  }
  'play' {
    Need-UE
    $p = Start-Process $UEEditor -PassThru -ArgumentList "`"$Proj`" $Map -game -windowed -ResX=1600 -ResY=900"
    "juego abierto: $Map (pid $($p.Id))"
  }
  'package' {
    # Development por defecto: Shipping apaga los AddOnScreenDebugMessage (ayuda de controles y "REBOBINANDO").
    Need-UE
    $out = Join-Path $Repo 'Game\Saved\Package'
    $log = Join-Path $Repo 'Game\Saved\Logs\tdc-package.log'
    if (Test-Path $out) { Remove-Item -Recurse -Force $out }
    New-Item -ItemType Directory -Force (Split-Path $log) | Out-Null
    & $UEUAT BuildCookRun -project="$Proj" -noP4 -platform=Win64 -clientconfig=$Config `
      -build -cook -stage -pak -iostore -prereqs -nodebuginfo -archive -archivedirectory="$out" -utf8output 2>&1 |
      Tee-Object -FilePath $log
    Check-Exit 'package'
    $game = Get-ChildItem $out -Directory | Select-Object -First 1
    if (-not $game) { throw "No hay build en $out (ver $log)" }
    $zipPath = Join-Path $Repo "Game\Saved\$Zip.zip"
    if (Test-Path $zipPath) { Remove-Item -Force $zipPath }
    Compress-Archive -Path (Join-Path $game.FullName '*') -DestinationPath $zipPath -CompressionLevel Optimal
    $mb = [math]::Round((Get-Item $zipPath).Length / 1MB, 1)
    "build: $(Redact $game.FullName)"
    "zip:   $(Redact $zipPath)  ($mb MB)"
    if ($mb -ge 1024) { Write-Warning 'El zip pesa 1 GB o más: la jam pide subirlo con butler.' }
  }
  'smoke' {
    # Prueba el zip tal como lo baja el jurado: lo descomprime en Game\Saved\PackageTest (ignorado), arranca el exe,
    # saca una captura, resume el log y lo cierra (o lo deja abierto con -Keep).
    if (-not $Task) { throw 'Falta -Task (ej. T-0006)' }
    $zipPath = Join-Path $Repo "Game\Saved\$Zip.zip"
    if (-not (Test-Path $zipPath)) { throw "No existe Game\Saved\$Zip.zip: correr antes tdc.ps1 package" }
    $dir = Join-Path $Repo 'Game\Saved\PackageTest'
    Get-CimInstance Win32_Process | Where-Object { $_.ExecutablePath -and $_.ExecutablePath -like "$dir\*" } |
      ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
    if (Test-Path $dir) { Remove-Item -Recurse -Force $dir }
    Expand-Archive -Path $zipPath -DestinationPath $dir
    $exe = Join-Path $dir 'TheDirectorsCut.exe'
    if (-not (Test-Path $exe)) { throw 'El zip no tiene TheDirectorsCut.exe en la raíz' }
    $before = Get-Date
    Start-Process $exe -ArgumentList "-windowed -ResX=1280 -ResY=720 -log -ExecCmds=`"HighResShot 1280x720`"" | Out-Null
    Start-Sleep -Seconds $Wait
    $running = @(Get-CimInstance Win32_Process | Where-Object { $_.ExecutablePath -and $_.ExecutablePath -like "$dir\*" })
    "seguía corriendo a los $Wait s: $(if ($running.Count) { 'sí' } else { 'NO (¿crash?)' })"
    $png = Get-ChildItem $dir -Recurse -Filter *.png -ErrorAction SilentlyContinue |
      Where-Object LastWriteTime -gt $before | Sort-Object LastWriteTime | Select-Object -Last 1
    if ($png) {
      $dest = Join-Path $Repo "docs\evidence\$Task"
      New-Item -ItemType Directory -Force $dest | Out-Null
      Copy-Item $png.FullName (Join-Path $dest "$Name.png")
      "captura: docs/evidence/$Task/$Name.png"
    } else { 'captura: no se generó' }
    if (-not $Keep) { $running | ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue } }
    else { 'juego abierto (-Keep)' }
    $gameLog = Get-ChildItem $dir -Recurse -Filter 'TheDirectorsCut.log' -ErrorAction SilentlyContinue | Select-Object -First 1
    if (-not $gameLog) { throw 'No hay log del juego (¿no arrancó?)' }
    "`n--- log del juego (extracto) ---"
    Redact ((Select-String -Path $gameLog.FullName -Pattern 'LoadMap|Fatal|Error:|Failed to (load|find)|Couldn''t find|M_TDCColor|SK_Director' |
      Select-Object -First 40 | ForEach-Object Line) -join "`n")
  }
}
