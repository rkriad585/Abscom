<#
.SYNOPSIS
    Build Abscom locally with Meson and Ninja, and optionally install it.

.DESCRIPTION
    Convenience wrapper around Meson for developing or setting up Abscom on
    your machine. Builds the static and shared libraries, runs the test
    suite, and (with -Install) installs the headers, libraries, and the
    pkg-config file to a prefix.

    Meson and Ninja are looked up on PATH; if Meson is installed as a Python
    package (e.g. `pip install meson ninja`), the module is used directly.

.EXAMPLE
    .\build.ps1

.EXAMPLE
    .\build.ps1 -Install -Prefix "C:\Abscom"

.EXAMPLE
    .\build.ps1 -Clean -BuildType release

.PARAMETER Prefix
    Install prefix, used with -Install. Defaults to the Meson default.

.PARAMETER BuildType
    Meson build type: debug, debugoptimized, release, or plain.

.PARAMETER Install
    Run `meson install` after a successful build.

.PARAMETER Clean
    Remove the build directory before configuring.

.PARAMETER SkipTests
    Do not run the test suite.

.PARAMETER BuildDir
    Build directory. Defaults to "build".
#>
param(
    [string]$Prefix = '',
    [ValidateSet('debug', 'debugoptimized', 'release', 'plain')]
    [string]$BuildType = 'release',
    [switch]$Install,
    [switch]$Clean,
    [switch]$SkipTests,
    [string]$BuildDir = 'build'
)

$ErrorActionPreference = 'Stop'
$RepoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path

if ($Prefix -and $Prefix -notmatch '^([A-Za-z]:[\\/]|/)') {
    Write-Error "Prefix must be an absolute path (got: '$Prefix')"
    exit 1
}

function Get-MesonCommand {
    if (Get-Command meson -ErrorAction SilentlyContinue) { return ,@('meson') }
    foreach ($py in @('python', 'python3', 'py')) {
        $cmd = Get-Command $py -ErrorAction SilentlyContinue
        if (-not $cmd) { continue }
        & $cmd.Source -c 'import mesonbuild' 2>$null
        if ($LASTEXITCODE -eq 0) { return ,@($cmd.Source, '-m', 'mesonbuild') }
    }
    return $null
}

function Get-NinjaCommand {
    if (Get-Command ninja -ErrorAction SilentlyContinue) { return 'ninja' }
    foreach ($py in @('python', 'python3', 'py')) {
        $cmd = Get-Command $py -ErrorAction SilentlyContinue
        if (-not $cmd) { continue }
        & $cmd.Source -c 'import ninja' 2>$null
        if ($LASTEXITCODE -eq 0) { return @($cmd.Source, '-m', 'ninja') }
    }
    return $null
}

$meson = Get-MesonCommand
if (-not $meson) {
    Write-Error 'Meson was not found. Install it with:  pip install meson ninja'
    exit 1
}
$ninja = Get-NinjaCommand
if (-not $ninja) {
    Write-Error 'Ninja was not found. Install it with:  pip install meson ninja'
    exit 1
}

$BuildDirAbs = Join-Path $RepoRoot $BuildDir

if ($Clean -and (Test-Path -LiteralPath $BuildDirAbs)) {
    Write-Host "Removing $BuildDirAbs ..."
    Remove-Item -LiteralPath $BuildDirAbs -Recurse -Force
}

if (Test-Path -LiteralPath $BuildDirAbs) {
    Write-Host "Reconfiguring $BuildDir ..."
    & $meson @('configure', $BuildDirAbs, "--buildtype=$BuildType")
    if ($LASTEXITCODE -ne 0) { exit 1 }
    if ($Prefix) {
        & $meson @('configure', $BuildDirAbs, "--prefix=$Prefix")
        if ($LASTEXITCODE -ne 0) { exit 1 }
    }
} else {
    Write-Host "Configuring $BuildDir (buildtype: $BuildType)..."
    $setupArgs = @('setup', $BuildDirAbs, "--buildtype=$BuildType")
    if ($Prefix) { $setupArgs += "--prefix=$Prefix" }
    & $meson @setupArgs
    if ($LASTEXITCODE -ne 0) { exit 1 }
}

Write-Host 'Compiling ...'
& $meson @('compile', '-C', $BuildDirAbs)
if ($LASTEXITCODE -ne 0) { exit 1 }

if (-not $SkipTests) {
    Write-Host 'Running tests ...'
    & $meson @('test', '-C', $BuildDirAbs)
    if ($LASTEXITCODE -ne 0) { exit 1 }
}

if ($Install) {
    Write-Host 'Installing ...'
    & $meson @('install', '-C', $BuildDirAbs)
    if ($LASTEXITCODE -ne 0) { exit 1 }
}

Write-Host ''
Write-Host 'Build complete.'
Write-Host "  Libraries:  $BuildDir\libabscom.a  (static)"
Write-Host "              $BuildDir\libabscom.dll / libabscom.so / libabscom.dylib  (shared)"
Write-Host "  Tests:      $BuildDir\tests\"
Write-Host "  Examples:   $BuildDir\examples\"
if ($Install) { Write-Host "  Installed to prefix: $Prefix" }
Write-Host ''
Write-Host 'Quick test of the installed library:'
Write-Host '  cc -std=c11 hello.c -I<prefix>/include -L<prefix>/lib -labscom -o hello'
Write-Host '  pkg-config --cflags --libs abscom   (set PKG_CONFIG_PATH to <prefix>/lib/pkgconfig)'
