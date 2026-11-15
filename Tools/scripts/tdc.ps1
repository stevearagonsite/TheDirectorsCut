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
#>
param(
  [Parameter(Position = 0, Mandatory = $true)]
  [ValidateSet('doctor', 'projectfiles', 'build', 'test', 'editor', 'python', 'blender', 'shot')]
  [string]$Command,
  [string]$Filter = 'TDC.',
  [string]$Script,
  [string]$Blend,
  [string]$Map = '/Game/Maps/L_Slice',
  [string]$Task,
  [string]$Name = 'shot',
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
    & $UECmd "$Proj" -run=pythonscript -script="$(Join-Path $Repo $Script)" -unattended -nosplash; Check-Exit 'python'
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
}
