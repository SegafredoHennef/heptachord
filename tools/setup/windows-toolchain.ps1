# Heptachord - Windows-Build-Toolchain
#
# Ausfuehren (normale Shell, KEIN Admin noetig):
#   powershell -ExecutionPolicy Bypass -File tools\setup\windows-toolchain.ps1
#
# Das Skript startet sich selbst mit Adminrechten neu (UAC-Bestaetigung).
# Es installiert nur, was fehlt: Git, CMake, Ninja, C++-Workload (MSVC + SDK).
# Reines ASCII: laeuft auch mit Windows PowerShell 5.1 ohne Encoding-Tricks.

$ErrorActionPreference = 'Stop'

function Test-IsAdmin {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($identity)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

if (-not (Test-IsAdmin)) {
    Write-Host "Starte mit Administratorrechten neu - bitte die UAC-Abfrage bestaetigen." -ForegroundColor Yellow
    $arguments = @(
        '-NoProfile',
        '-ExecutionPolicy', 'Bypass',
        '-File', ('"{0}"' -f $PSCommandPath)
    )
    Start-Process -FilePath 'powershell.exe' -ArgumentList $arguments -Verb RunAs
    exit 0
}

$Total = 5

function Write-Step {
    param([int]$Number, [string]$Text)
    Write-Host ''
    Write-Host ('[{0}/{1}] {2}' -f $Number, $Total, $Text) -ForegroundColor Cyan
}

function Get-WingetPath {
    $command = Get-Command winget.exe -ErrorAction SilentlyContinue
    if ($command) { return $command.Source }
    return $null
}

function Install-WingetPackage {
    param([string]$Id, [string]$DisplayName, [string]$Winget)

    Write-Host ('  installiere {0} ...' -f $DisplayName)
    $wingetArgs = @(
        'install', '--id', $Id, '-e', '--source', 'winget', '--silent',
        '--disable-interactivity',
        '--accept-package-agreements', '--accept-source-agreements'
    )
    & $Winget @wingetArgs
    if ($LASTEXITCODE -ne 0) {
        throw ('{0}: winget endete mit Code {1}' -f $DisplayName, $LASTEXITCODE)
    }
}

Write-Host 'Heptachord - Build-Toolchain' -ForegroundColor Green

$winget = Get-WingetPath
if (-not $winget) {
    throw 'winget.exe nicht gefunden. Bitte "App Installer" aus dem Microsoft Store installieren.'
}

# --- 1/5 Git ---------------------------------------------------------------
Write-Step 1 'Git'
if (Get-Command git.exe -ErrorAction SilentlyContinue) {
    Write-Host ('  vorhanden: {0}' -f ((git --version) -join ' '))
} else {
    Install-WingetPackage -Id 'Git.Git' -DisplayName 'Git' -Winget $winget
}

# --- 2/5 CMake -------------------------------------------------------------
Write-Step 2 'CMake'
if (Get-Command cmake.exe -ErrorAction SilentlyContinue) {
    Write-Host ('  vorhanden: {0}' -f ((cmake --version) | Select-Object -First 1))
} else {
    Install-WingetPackage -Id 'Kitware.CMake' -DisplayName 'CMake' -Winget $winget
}

# --- 3/5 Ninja -------------------------------------------------------------
Write-Step 3 'Ninja'
if (Get-Command ninja.exe -ErrorAction SilentlyContinue) {
    Write-Host ('  vorhanden: {0}' -f (ninja --version))
} else {
    Install-WingetPackage -Id 'Ninja-build.Ninja' -DisplayName 'Ninja' -Winget $winget
}

# --- 4/5 C++-Workload ------------------------------------------------------
Write-Step 4 'C++-Workload (MSVC + Windows SDK)'

$installerRoot = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer'
$vswhere = Join-Path $installerRoot 'vswhere.exe'
if (-not (Test-Path $vswhere)) {
    throw 'vswhere.exe fehlt. Bitte "Microsoft Visual Studio Installer" installieren.'
}

function Find-Compiler {
    $hits = & $vswhere -latest -products * `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -find 'VC\Tools\MSVC\*\bin\Hostx64\x64\cl.exe' 2>$null
    if ($hits) { return ($hits | Select-Object -First 1) }
    return $null
}

$compiler = Find-Compiler
if ($compiler) {
    Write-Host ('  vorhanden: {0}' -f $compiler)
} else {
    $instances = & $vswhere -products * -format json 2>$null | ConvertFrom-Json
    $target = $instances | Where-Object { $_.installationPath } | Select-Object -First 1
    if (-not $target) {
        throw 'Keine Visual-Studio-Instanz gefunden. Bitte "Visual Studio Build Tools" installieren.'
    }

    $setup = Join-Path $installerRoot 'setup.exe'
    Write-Host ('  ergaenze Workload in: {0}' -f $target.installationPath)
    Write-Host '  Download ca. 3 GB, Dauer 5-20 Minuten. Bitte warten.'
    $setupArgs = @(
        'modify',
        ('--installPath "{0}"' -f $target.installationPath),
        '--add', 'Microsoft.VisualStudio.Workload.VCTools',
        '--includeRecommended',
        '--quiet', '--norestart', '--wait'
    )
    $process = Start-Process -FilePath $setup -ArgumentList $setupArgs -Wait -PassThru
    if ($process.ExitCode -eq 3010) {
        Write-Host '  fertig - ein Neustart wird empfohlen.' -ForegroundColor Yellow
    } elseif ($process.ExitCode -ne 0) {
        throw ('VS-Installer endete mit Code {0}.' -f $process.ExitCode)
    }
}

# --- 5/5 Pruefen -----------------------------------------------------------
Write-Step 5 'Pruefen'

$compiler = Find-Compiler
$missing = @()
if (-not (Get-Command git.exe   -ErrorAction SilentlyContinue)) { $missing += 'git' }
if (-not (Get-Command cmake.exe -ErrorAction SilentlyContinue)) { $missing += 'cmake' }
if (-not (Get-Command ninja.exe -ErrorAction SilentlyContinue)) { $missing += 'ninja' }
if (-not $compiler) { $missing += 'MSVC (cl.exe)' }

if ($missing.Count -gt 0) {
    Write-Host ('  noch offen: {0}' -f ($missing -join ', ')) -ForegroundColor Yellow
    Write-Host '  Neue Shell oeffnen (PATH) und erneut pruefen.'
    Read-Host 'Enter zum Schliessen'
    exit 1
}

Write-Host ''
Write-Host 'Alles vorhanden:' -ForegroundColor Green
Write-Host ('  git    {0}' -f ((git --version) -join ' '))
Write-Host ('  cmake  {0}' -f ((cmake --version) | Select-Object -First 1))
Write-Host ('  ninja  {0}' -f (ninja --version))
Write-Host ('  cl     {0}' -f $compiler)
Write-Host ''
Write-Host 'Naechster Schritt (neue Shell):'
Write-Host '  cd C:\opencode-projekte\heptachord'
Write-Host '  cmake --preset vs'
Read-Host 'Enter zum Schliessen'
