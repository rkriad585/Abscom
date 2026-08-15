<#
.SYNOPSIS
    One-line installer for Abscom, a C11 library of data structures and a
    Python-inspired dynamic runtime.

.DESCRIPTION
    Downloads a prebuilt release asset for the current platform when the
    matching GitHub Release exists, and otherwise downloads the Abscom source
    from GitHub (or reuses the local checkout when the script runs from inside
    a repository clone), builds it, and installs the headers, libraries, and
    pkg-config file to a prefix.

    The source path uses Meson/Ninja when available and falls back to compiling
    the static library directly with gcc/clang or MSVC cl.exe. Set the
    ABSCOM_ASSET_URL environment variable to an absolute download URL (or a
    local file path) to override the prebuilt asset location. Use -SelfUninstall
    to remove everything it installed.

.EXAMPLE
    irm https://raw.githubusercontent.com/rkriad585/Abscom/main/installer.ps1 | iex

.EXAMPLE
    (Invoke-RestMethod https://raw.githubusercontent.com/rkriad585/Abscom/main/installer.ps1) + " -SelfUninstall" | iex

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File .\installer.ps1 -Prefix "C:\Abscom"

.PARAMETER Prefix
    Install prefix. Defaults to "$HOME\abscom".

.PARAMETER BuildType
    Meson build type: debug, debugoptimized, release, or plain.

.PARAMETER SelfUninstall
    Remove installed Abscom files and the cache directory.

.PARAMETER SkipTests
    Do not run the test suite during the build.

.PARAMETER ForceDirect
    Compile the static library directly with gcc/clang or MSVC cl.exe instead
    of using Meson/Ninja.

.PARAMETER CacheDir
    Where downloaded sources, the build directory, and the uninstall manifest
    live. Defaults to "$env:LOCALAPPDATA\abscom".
#>
param(
    [string]$Prefix = '',
    [ValidateSet('debug', 'debugoptimized', 'release', 'plain')]
    [string]$BuildType = 'release',
    [switch]$SelfUninstall,
    [switch]$SkipTests,
    [switch]$ForceDirect,
    [string]$CacheDir = ''
)

$ErrorActionPreference = 'Stop'
$Owner = 'rkriad585'
$Repo = 'Abscom'
$Branch = 'main'
$Version = '0.1.0'

$localSrc = ''
if ($PSScriptRoot -and (Test-Path -LiteralPath (Join-Path $PSScriptRoot 'meson.build'))) {
    $localSrc = $PSScriptRoot
}

if (-not $CacheDir) { $CacheDir = Join-Path $env:LOCALAPPDATA 'abscom' }

$buildDir = Join-Path $CacheDir 'build'
$manifest = Join-Path $CacheDir 'manifest.txt'
$srcRoot = Join-Path $CacheDir 'src\Abscom-main'

function Write-Step([string]$Message) { Write-Host "[abscom] $Message" }

function Write-InstallSummary {
    Write-Host ''
    Write-Step "Abscom installed."
    Write-Step "  prefix   : $Prefix"
    Write-Step "  headers  : $Prefix\include\abscom"
    Write-Step "  library  : $Prefix\lib"
    Write-Step "  pkg-config: set PKG_CONFIG_PATH=$pkgDir then:  pkg-config --cflags --libs abscom"
    Write-Host ''
    Write-Step 'Compile a quick test:'
    Write-Step "  cc -std=c11 hello.c -I$Prefix\include -L$Prefix\lib -labscom -o hello"
    Write-Step 'Uninstall:'
    Write-Step "  (Invoke-RestMethod https://raw.githubusercontent.com/$Owner/$Repo/main/installer.ps1) + `" -SelfUninstall`" | iex"
}

function Get-CanonicalPath([string]$Path) {
    if ([string]::IsNullOrWhiteSpace($Path)) { return $Path }
    try { return (Get-Item -LiteralPath $Path -ErrorAction Stop).FullName } catch { return $Path }
}

function Remove-EmptyDir([string]$Path) {
    if ([string]::IsNullOrWhiteSpace($Path) -or -not (Test-Path -LiteralPath $Path)) { return }
    try { [System.IO.Directory]::Delete($Path, $false) } catch { }
}

function Remove-Installed {
    if (Test-Path -LiteralPath $manifest) {
        Write-Step 'Removing installed Abscom files ...'
        $paths = @(Get-Content -LiteralPath $manifest | Where-Object { $_ })
        foreach ($p in $paths) {
            if (Test-Path -LiteralPath $p) { Remove-Item -LiteralPath $p -Force -ErrorAction SilentlyContinue }
        }
        $protected = @('/', '/usr', '/usr/local', '/opt', '/home', '/root')
        $normPrefix = Get-CanonicalPath $Prefix
        foreach ($p in $paths | ForEach-Object { Split-Path -Parent $_ } | Sort-Object -Unique | Sort-Object -Descending) {
            $dir = Get-CanonicalPath $p
            while ($dir -and $dir -ne $normPrefix -and ($dir -like "$normPrefix*")) {
                Remove-EmptyDir $dir
                $dir = Split-Path -Parent $dir
            }
        }
        if ($protected -notcontains $normPrefix) {
            Remove-EmptyDir $normPrefix
        }
    }
    if (Test-Path -LiteralPath $CacheDir) { Remove-Item -LiteralPath $CacheDir -Recurse -Force -ErrorAction SilentlyContinue }
    Write-Step 'Abscom uninstalled.'
}

if ($SelfUninstall) {
    $storedPrefix = Join-Path $CacheDir 'prefix.txt'
    if (-not $Prefix -and (Test-Path -LiteralPath $storedPrefix)) {
        $Prefix = (Get-Content -LiteralPath $storedPrefix | Select-Object -First 1).Trim()
    }
    if (-not $Prefix) { $Prefix = Join-Path $HOME 'abscom' }
    Remove-Installed
    return
}

if (-not $Prefix) { $Prefix = Join-Path $HOME 'abscom' }

$incDir = Join-Path $Prefix 'include\abscom'
$libDir = Join-Path $Prefix 'lib'
$pkgDir = Join-Path $libDir 'pkgconfig'

# ---- prebuilt release asset (fast path) ----
$osId = ''
if ([System.Runtime.InteropServices.RuntimeInformation]::IsOSPlatform([System.Runtime.InteropServices.OSPlatform]::Windows)) { $osId = 'windows' }
elseif ([System.Runtime.InteropServices.RuntimeInformation]::IsOSPlatform([System.Runtime.InteropServices.OSPlatform]::Linux)) { $osId = 'linux' }
elseif ([System.Runtime.InteropServices.RuntimeInformation]::IsOSPlatform([System.Runtime.InteropServices.OSPlatform]::OSX)) { $osId = 'macos' }
$archId = ''
switch ([System.Runtime.InteropServices.RuntimeInformation]::OSArchitecture) {
    'X64'   { $archId = 'x86_64' }
    'Arm64' { $archId = 'arm64' }
}

$installedFromAsset = $false
if ($osId -and $archId) {
    $asset = "abscom-$osId-$archId.zip"
    $assetLocal = Join-Path $CacheDir $asset
    New-Item -ItemType Directory -Force -Path (Split-Path $assetLocal -Parent) | Out-Null
    $urls = @()
    if ($env:ABSCOM_ASSET_URL) { $urls += $env:ABSCOM_ASSET_URL }
    else {
        $urls += "https://github.com/$Owner/$Repo/releases/download/v$Version/$asset"
        $urls += "https://github.com/$Owner/$Repo/releases/latest/download/$asset"
    }

    $downloaded = $false
    foreach ($u in $urls) {
        Write-Step "Downloading prebuilt asset: $u"
        try {
            if (Test-Path -LiteralPath $u) {
                Copy-Item -LiteralPath $u -Destination $assetLocal -Force
                $downloaded = $true
            }
            elseif (Get-Command curl.exe -ErrorAction SilentlyContinue) {
                & curl.exe -fsSL $u -o $assetLocal
                if ($LASTEXITCODE -eq 0) { $downloaded = $true }
            }
            else {
                Invoke-WebRequest -Uri $u -OutFile $assetLocal -ErrorAction Stop
                $downloaded = $true
            }
        } catch { }
        if ($downloaded) { break }
    }

    if ($downloaded -and (Test-Path -LiteralPath $assetLocal)) {
        Write-Step 'Installing from prebuilt release asset ...'
        $prebuiltDir = Join-Path $CacheDir 'prebuilt'
        if (Test-Path -LiteralPath $prebuiltDir) { Remove-Item -LiteralPath $prebuiltDir -Recurse -Force }
        New-Item -ItemType Directory -Force -Path $prebuiltDir | Out-Null
        Expand-Archive -LiteralPath $assetLocal -DestinationPath $prebuiltDir -Force
        Remove-Item -LiteralPath $assetLocal -Force

        New-Item -ItemType Directory -Force -Path $incDir, $libDir, $pkgDir | Out-Null
        Copy-Item -Recurse -Force (Join-Path $prebuiltDir 'include\abscom\*') $incDir
        Copy-Item -Recurse -Force (Join-Path $prebuiltDir 'lib\*') $libDir
        $binSrc = Join-Path $prebuiltDir 'bin'
        if (Test-Path -LiteralPath $binSrc) {
            $binDir = Join-Path $Prefix 'bin'
            New-Item -ItemType Directory -Force -Path $binDir | Out-Null
            Copy-Item -Recurse -Force (Join-Path $binSrc '*') $binDir
        }

        $pcPath = Join-Path $pkgDir 'abscom.pc'
        (Get-Content -LiteralPath $pcPath) -replace '^prefix=.*', "prefix=$Prefix" |
            Set-Content -LiteralPath $pcPath -Encoding ASCII

        $manifestEntries = @()
        $manifestEntries += @(Get-ChildItem -LiteralPath $incDir -Recurse -File | ForEach-Object { $_.FullName })
        $manifestEntries += @(Get-ChildItem -LiteralPath $libDir -Recurse -File | ForEach-Object { $_.FullName })
        $binDir = Join-Path $Prefix 'bin'
        if (Test-Path -LiteralPath $binDir) {
            $manifestEntries += @(Get-ChildItem -LiteralPath $binDir -Recurse -File | ForEach-Object { $_.FullName })
        }
        $manifestEntries | Sort-Object -Unique | Set-Content -LiteralPath $manifest -Encoding ASCII
        Set-Content -LiteralPath (Join-Path $CacheDir 'prefix.txt') -Value $Prefix -Encoding ASCII

        $installedFromAsset = $true
    }
    else {
        if (Test-Path -LiteralPath $assetLocal) { Remove-Item -LiteralPath $assetLocal -Force }
        Write-Step 'Prebuilt asset not available; building from source.'
    }
}

if ($installedFromAsset) {
    Write-InstallSummary
    return
}

# ---- source: local checkout or download from GitHub ----
$srcDir = $localSrc
if (-not $srcDir) {
    Write-Step "Downloading $Owner/$Repo@$Branch ..."
    New-Item -ItemType Directory -Force -Path (Split-Path $srcRoot -Parent) | Out-Null
    $zip = Join-Path $CacheDir "abscom-$Branch.zip"
    $url = "https://github.com/$Owner/$Repo/archive/refs/heads/$Branch.zip"
    if (Get-Command curl.exe -ErrorAction SilentlyContinue) {
        & curl.exe -fsSL $url -o $zip
        if ($LASTEXITCODE -ne 0) { Write-Error "Download failed: $url" }
    } else {
        Invoke-WebRequest -Uri $url -OutFile $zip
    }
    if (Test-Path -LiteralPath $srcRoot) { Remove-Item -LiteralPath $srcRoot -Recurse -Force }
    Expand-Archive -Path $zip -DestinationPath (Split-Path $srcRoot -Parent) -Force
    Remove-Item -LiteralPath $zip -Force
    $srcDir = $srcRoot
}

if (-not (Test-Path -LiteralPath (Join-Path $srcDir 'meson.build'))) {
    Write-Error "Source directory is missing meson.build: $srcDir"
}

# ---- build & install ----
$mesonCmd = $null
if (Get-Command meson -ErrorAction SilentlyContinue) { $mesonCmd = @('meson') }
else {
    foreach ($py in @('python', 'python3', 'py')) {
        $cmd = Get-Command $py -ErrorAction SilentlyContinue
        if (-not $cmd) { continue }
        & $cmd.Source -c 'import mesonbuild' 2>$null
        if ($LASTEXITCODE -eq 0) { $mesonCmd = @($cmd.Source, '-m', 'mesonbuild'); break }
    }
}

$haveNinja = [bool](Get-Command ninja -ErrorAction SilentlyContinue)

if (-not $ForceDirect -and $mesonCmd -and $haveNinja) {
    Write-Step "Building with Meson ($BuildType) ..."
    New-Item -ItemType Directory -Force -Path (Split-Path $buildDir -Parent) | Out-Null
    if (Test-Path -LiteralPath $buildDir) { Remove-Item -LiteralPath $buildDir -Recurse -Force }
    & $mesonCmd @('setup', $buildDir, $srcDir, "--buildtype=$BuildType", "--prefix=$Prefix", '--libdir=lib')
    if ($LASTEXITCODE -ne 0) { Write-Error 'meson setup failed' }
    & $mesonCmd @('compile', '-C', $buildDir)
    if ($LASTEXITCODE -ne 0) { Write-Error 'meson compile failed' }
    if (-not $SkipTests) {
        Write-Step 'Running tests ...'
        & $mesonCmd @('test', '-C', $buildDir)
        if ($LASTEXITCODE -ne 0) { Write-Error 'meson test failed' }
    }
    & $mesonCmd @('install', '-C', $buildDir)
    if ($LASTEXITCODE -ne 0) { Write-Error 'meson install failed' }

    Write-Step 'Recording install manifest ...'
    $raw = (& $mesonCmd @('introspect', '--installed', $buildDir) 2>$null | Out-String)
    try {
        $obj = $raw | ConvertFrom-Json
        $normPrefix = $Prefix.Replace('\', '/')
        $installed = @($obj.PSObject.Properties | ForEach-Object { $_.Value } | Where-Object {
                $_.Replace('\', '/') -like "$normPrefix/*"
            })
    } catch {
        $installed = @()
    }
    if ($installed.Count -eq 0) { Write-Error 'Could not determine installed files for the uninstall manifest.' }
    $installed | Set-Content -LiteralPath $manifest -Encoding ASCII
    Set-Content -LiteralPath (Join-Path $CacheDir 'prefix.txt') -Value $Prefix -Encoding ASCII
}
else {
    Write-Step 'Meson/Ninja not found; compiling the static library directly ...'
    $objDir = Join-Path $CacheDir 'direct\obj'
    if (Test-Path -LiteralPath $objDir) { Remove-Item -LiteralPath $objDir -Recurse -Force }
    New-Item -ItemType Directory -Force -Path $objDir | Out-Null
    New-Item -ItemType Directory -Force -Path $incDir, $libDir, $pkgDir | Out-Null

    $srcInc = Join-Path $srcDir 'include'
    $srcFiles = @(Get-ChildItem -Path (Join-Path $srcDir 'src') -Filter '*.c')
    $gcc = Get-Command gcc -ErrorAction SilentlyContinue
    $cl = Get-Command cl -ErrorAction SilentlyContinue

    $objList = @()
    if ($gcc) {
        foreach ($src in $srcFiles) {
            $obj = Join-Path $objDir ($src.BaseName + '.o')
            & $gcc.Source -std=c11 -O2 -DABS_BUILDING_LIBRARY -I $srcInc -c $src.FullName -o $obj
            if ($LASTEXITCODE -ne 0) { Write-Error "gcc failed on $($src.Name)" }
            $objList += $obj
        }
        & ar rcs (Join-Path $libDir 'libabscom.a') $objList
        if ($LASTEXITCODE -ne 0) { Write-Error 'ar failed' }
    }
    elseif ($cl) {
        foreach ($src in $srcFiles) {
            $obj = Join-Path $objDir ($src.BaseName + '.obj')
            & $cl.Source /nologo /std:c11 /O2 /DABS_BUILDING_LIBRARY "/I$srcInc" /c $src.FullName "/Fo$obj"
            if ($LASTEXITCODE -ne 0) { Write-Error "cl failed on $($src.Name)" }
            $objList += $obj
        }
        & lib /nologo "/OUT:$libDir\abscom.lib" $objList
        if ($LASTEXITCODE -ne 0) { Write-Error 'lib failed' }
    }
    else {
        Write-Error 'No C compiler found. Install gcc, clang, or MSVC and run this installer again.'
    }

    Copy-Item -Path (Join-Path $srcInc 'abscom\*.h') -Destination $incDir -Force

    $osIsWindows = [System.Runtime.InteropServices.RuntimeInformation]::IsOSPlatform([System.Runtime.InteropServices.OSPlatform]::Windows)
    $privateLibs = if ($osIsWindows) { 'Libs.private: -lws2_32' } else { '' }
    $pc = @(
        "prefix=$Prefix"
        'exec_prefix=${prefix}'
        'libdir=${exec_prefix}/lib'
        'includedir=${prefix}/include'
        ''
        'Name: abscom'
        'Description: A C11 library of reusable data structures and a Python-inspired dynamic runtime'
        "Version: $Version"
        'Libs: -L${libdir} -labscom'
        $privateLibs
        'Cflags: -I${includedir}'
    )
    $pcPath = Join-Path $pkgDir 'abscom.pc'
    Set-Content -LiteralPath $pcPath -Value $pc -Encoding ASCII

    @("$libDir\libabscom.a", $pcPath) + @(Get-ChildItem -LiteralPath $incDir -Filter '*.h' | ForEach-Object { $_.FullName }) |
        Set-Content -LiteralPath $manifest -Encoding ASCII
    Set-Content -LiteralPath (Join-Path $CacheDir 'prefix.txt') -Value $Prefix -Encoding ASCII
}

Write-InstallSummary
