[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$pinnedVersion = '2026.07.29'
$localVcpkg = Join-Path $projectRoot '.deps\vcpkg'

if ($env:VCPKG_ROOT -and (Test-Path (Join-Path $env:VCPKG_ROOT 'scripts\buildsystems\vcpkg.cmake'))) {
    Write-Host "Using VCPKG_ROOT=$env:VCPKG_ROOT"
    return
}

if (-not (Test-Path (Join-Path $localVcpkg '.git'))) {
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $localVcpkg) | Out-Null
    git clone --branch $pinnedVersion --depth 1 https://github.com/microsoft/vcpkg.git $localVcpkg
}

$currentTag = git -C $localVcpkg describe --tags --exact-match 2>$null
if ($LASTEXITCODE -ne 0 -or $currentTag -ne $pinnedVersion) {
    git -C $localVcpkg fetch --depth 1 origin "refs/tags/$pinnedVersion`:refs/tags/$pinnedVersion"
    git -C $localVcpkg checkout --detach $pinnedVersion
}

if (-not (Test-Path (Join-Path $localVcpkg 'vcpkg.exe'))) {
    & (Join-Path $localVcpkg 'bootstrap-vcpkg.bat') -disableMetrics
}
$env:VCPKG_ROOT = $localVcpkg
Write-Host "VCPKG_ROOT=$env:VCPKG_ROOT"
