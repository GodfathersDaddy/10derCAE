[CmdletBinding()]
param([string]$Configuration = 'Release')

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
& (Join-Path $PSScriptRoot 'build.ps1') -Configuration $Configuration
$buildDir = Join-Path $projectRoot 'build\windows-x64'
$stage = Join-Path $projectRoot 'out\10derCAE'
$archive = Join-Path $projectRoot 'out\10derCAE-windows-x64.zip'

if (Test-Path $stage) { Remove-Item -LiteralPath $stage -Recurse -Force }
New-Item -ItemType Directory -Force -Path $stage | Out-Null
cmake --install $buildDir --config $Configuration --prefix $stage

$manifestRoot = Join-Path $buildDir 'vcpkg_installed\x64-windows'
$qtDeploy = Get-ChildItem -Path $manifestRoot -Filter 'windeployqt.exe' -Recurse | Select-Object -First 1
if ($qtDeploy) {
    & $qtDeploy.FullName --release --no-translations (Join-Path $stage '10derCAE.exe')
} else {
    foreach ($plugin in @('platforms', 'styles', 'iconengines', 'imageformats')) {
        $source = Join-Path $manifestRoot "Qt6\plugins\$plugin"
        if (Test-Path $source) { Copy-Item -LiteralPath $source -Destination (Join-Path $stage $plugin) -Recurse -Force }
    }
}

$tripletBin = Join-Path $manifestRoot 'bin'
Get-ChildItem -Path $tripletBin -Filter '*.dll' | Copy-Item -Destination $stage -Force
if (Test-Path $archive) { Remove-Item -LiteralPath $archive -Force }
Compress-Archive -Path (Join-Path $stage '*') -DestinationPath $archive -CompressionLevel Optimal
Write-Host "Portable package: $archive"
