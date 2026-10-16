[CmdletBinding()]
param(
    [ValidateSet('de', 'en')][string]$Language = 'de',
    [ValidateSet('100', '125', '150')][string]$DpiPercent = '100',
    [string]$OutputDirectory = ''
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
if (-not $OutputDirectory) { $OutputDirectory = Join-Path $projectRoot 'out\screenshots' }
New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
$exe = Join-Path $projectRoot 'build\windows-x64\Release\10derCAE.exe'
if (-not (Test-Path $exe)) { & (Join-Path $PSScriptRoot 'build.ps1') -SkipTests }

foreach ($workspace in @('start','part','sketch','assembly','drawing','fem','dynamics','system')) {
    $target = Join-Path $OutputDirectory "$Language-$workspace-1440x920-$DpiPercent.bmp"
    $oldScale = $env:QT_SCALE_FACTOR
    $env:QT_SCALE_FACTOR = [string]([int]$DpiPercent / 100)
    & $exe --lang $Language --workspace $workspace --size 1440x920 --capture $target
    $env:QT_SCALE_FACTOR = $oldScale
    if ($LASTEXITCODE -ne 0) { throw "Screenshot failed for $workspace (exit $LASTEXITCODE)." }
}
