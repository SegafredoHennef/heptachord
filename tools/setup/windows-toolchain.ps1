<#
.SYNOPSIS
    Richtet die Windows-Build-Toolchain für Heptachord ein.

.DESCRIPTION
    Installiert Git, CMake, Ninja und die C++-Workload der Visual Studio Build
    Tools 2026 (MSVC v143 + Windows SDK). Idempotent: bereits vorhandene
    Komponenten werden übersprungen.

    Das Skript muss ERHÖHT (Administrator) laufen, weil die C++-Workload
    maschinenweit installiert wird. Download ca. 3 GB, Dauer 5–20 Minuten.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File tools\setup\windows-toolchain.ps1
#>

$ErrorActionPreference = 'Stop'

function Test-Admin {
    $id = [Security.Principal.WindowsIdentity]::GetCurrent()
    (New-Object Security.Principal.WindowsPrincipal $id).IsInRole(
        [Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Write-Step($n, $total, $text) {
    Write-Host ""
    Write-Host "[$n/$total] $text" -ForegroundColor Cyan
}

if (-not (Test-Admin)) {
    Write-Host "Dieses Skript braucht Administratorrechte." -ForegroundColor Yellow
    Write-Host "Rechtsklick auf PowerShell > 'Als Administrator ausführen', dann erneut:"
    Write-Host "  powershell -ExecutionPolicy Bypass -File `"$PSCommandPath`""
    exit 1
}

$steps = 5
Write-Host "Heptachord — Build-Toolchain" -ForegroundColor Green
Write-Host "Dieses Skript installiert Git, CMake, Ninja und MSVC (C++)."

Write-Step 1 $steps "Git"
if (Get-Command git -ErrorAction SilentlyContinue) {
    Write-Host "  vorhanden: $((git --version))"
} else {
    Write-Host "  installiere Git ..."
    winget install --id Git.Git -e --source winget --silent `
        --accept-package-agreements --accept-source-agreements
}

Write-Step 2 $steps "CMake"
if (Get-Command cmake -ErrorAction SilentlyContinue) {
    Write-Host "  vorhanden: $((cmake --version)[0])"
} else {
    Write-Host "  installiere CMake ..."
    winget install --id Kitware.CMake -e --source winget --silent `
        --accept-package-agreements --accept-source-agreements
}

Write-Step 3 $steps "Ninja"
if (Get-Command ninja -ErrorAction SilentlyContinue) {
    Write-Host "  vorhanden: $((ninja --version))"
} else {
    Write-Host "  installiere Ninja ..."
    winget install --id Ninja-build.Ninja -e --source winget --silent `
        --accept-package-agreements --accept-source-agreements
}

Write-Step 4 $steps "C++-Workload der Build Tools 2026 (MSVC v143 + Windows SDK)"
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path $vswhere)) {
    throw "vswhere.exe fehlt — bitte 'Visual Studio Installer' installieren."
}

$cl = & $vswhere -latest -products * `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -find 'VC\Tools\MSVC\*\bin\Hostx64\x64\cl.exe' 2>$null | Select-Object -First 1

if ($cl) {
    Write-Host "  vorhanden: $cl"
} else {
    $setup = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\setup.exe'
    $instances = & $vswhere -products * -format json | ConvertFrom-Json
    $target = $instances | Where-Object { $_.installationPath } | Select-Object -First 1
    if (-not $target) {
        throw "Keine Visual-Studio-Instanz gefunden. Bitte 'Build Tools für Visual Studio 2026' installieren."
    }
    Write-Host "  ergänze Workload in $($target.installationPath) ..."
    Write-Host "  (Download ca. 3 GB — das dauert)"
    $args = @(
        'modify',
        '--installPath', "`"$($target.installationPath)`"",
        '--add', 'Microsoft.VisualStudio.Workload.VCTools',
        '--includeRecommended',
        '--quiet', '--norestart', '--wait'
    )
    $p = Start-Process -FilePath $setup -ArgumentList $args -Wait -PassThru
    if ($p.ExitCode -eq 3010) {
        Write-Host "  Installation abgeschlossen — Neustart empfohlen." -ForegroundColor Yellow
    } elseif ($p.ExitCode -ne 0) {
        throw "VS-Installer endete mit Code $($p.ExitCode)."
    }
}

Write-Step 5 $steps "Prüfen"
$cl = & $vswhere -latest -products * `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -find 'VC\Tools\MSVC\*\bin\Hostx64\x64\cl.exe' 2>$null | Select-Object -First 1

$missing = @()
if (-not (Get-Command git   -ErrorAction SilentlyContinue)) { $missing += 'git' }
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) { $missing += 'cmake' }
if (-not (Get-Command ninja -ErrorAction SilentlyContinue)) { $missing += 'ninja' }
if (-not $cl) { $missing += 'MSVC (cl.exe)' }

if ($missing.Count -gt 0) {
    Write-Host "  noch offen: $($missing -join ', ')" -ForegroundColor Yellow
    Write-Host "  Neue Shell öffnen (PATH) und erneut prüfen."
    exit 1
}

Write-Host ""
Write-Host "Alles vorhanden:" -ForegroundColor Green
Write-Host "  git    $((git --version))"
Write-Host "  cmake  $((cmake --version)[0])"
Write-Host "  ninja  $((ninja --version))"
Write-Host "  cl     $cl"
Write-Host ""
Write-Host "Nächster Schritt: neue Shell öffnen, dann"
Write-Host "  cd C:\opencode-projekte\heptachord"
Write-Host "  git init"
