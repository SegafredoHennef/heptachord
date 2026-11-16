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

function Find-InstallPath {
    # vswhere kennt nur vollstaendig registrierte Instanzen. Genau daran ist
    # die erste Fassung gescheitert: die Build Tools standen in den Paketen,
    # vswhere lieferte aber eine leere Liste. Deshalb drei Stufen.
    $paths = & $vswhere -products * -property installationPath 2>$null
    if ($paths) { return ($paths | Select-Object -First 1) }

    $instances = Join-Path $env:ProgramData 'Microsoft\VisualStudio\Packages\_Instances'
    if (Test-Path $instances) {
        $states = Get-ChildItem $instances -Recurse -Filter 'state.json' -ErrorAction SilentlyContinue
        foreach ($state in $states) {
            try {
                $json = Get-Content -Raw -LiteralPath $state.FullName | ConvertFrom-Json
                if ($json.installationPath) { return $json.installationPath }
            } catch {
                # beschaedigte state.json ueberspringen
            }
        }
    }

    $known = @(
        (Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\18\BuildTools'),
        (Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\2022\BuildTools'),
        (Join-Path $env:ProgramFiles 'Microsoft Visual Studio\2022\BuildTools')
    )
    foreach ($candidate in $known) {
        if (Test-Path $candidate) { return $candidate }
    }
    return $null
}

$compiler = Find-Compiler
if ($compiler) {
    Write-Host ('  vorhanden: {0}' -f $compiler)
} else {
    $target = Find-InstallPath
    if (-not $target) {
        throw 'Keine Visual-Studio-Instanz gefunden. Bitte "Visual Studio Build Tools" installieren.'
    }

    $free = (Get-PSDrive C).Free
    if ($free -lt 7GB) {
        throw ('Zu wenig Platz auf C: {0:N1} GB frei, die C++-Workload braucht ca. 6,6 GB.' -f ($free / 1GB))
    }

    $setup = Join-Path $installerRoot 'setup.exe'
    Write-Host ('  ergaenze Workload in: {0}' -f $target)
    Write-Host ('  freier Platz: {0:N1} GB' -f ($free / 1GB))
    Write-Host '  Download ca. 3 GB, Dauer 5-20 Minuten.'

    # Der Installer ab VS 2026 kennt --wait nicht mehr und koppelt sich ab.
    # Deshalb eine Argumentzeile als String (Array verliert die Quotes am
    # Leerzeichen in "Program Files (x86)") und danach auf cl.exe warten.
    $registered = & $vswhere -products * -property installationPath 2>$null
    if ($registered) {
        $argLine = 'modify --installPath "' + $target + '" ' +
                   '--add Microsoft.VisualStudio.Workload.VCTools ' +
                   '--includeRecommended --quiet --norestart'
    } else {
        # Verzeichnis existiert, ist dem Installer aber unbekannt - etwa nach
        # manuellem Loeschen des Paket-Caches. Dann neu registrieren statt
        # "modify" (das meldet sonst "Produkt nicht gefunden").
        Write-Host '  Instanz ist dem Installer unbekannt - registriere neu.'
        $argLine = 'install --installPath "' + $target + '" ' +
                   '--productId Microsoft.VisualStudio.Product.BuildTools ' +
                   '--channelUri https://aka.ms/vs/18/stable/channel ' +
                   '--add Microsoft.VisualStudio.Workload.VCTools ' +
                   '--includeRecommended --quiet --norestart'
    }
    $process = Start-Process -FilePath $setup -ArgumentList $argLine -PassThru
    Write-Host ('  Installer gestartet (PID {0}), warte auf cl.exe ...' -f $process.Id)

    $vcvars = Join-Path $target 'VC\Auxiliary\Build\vcvars64.bat'
    for ($i = 0; $i -lt 160 -and -not (Test-Path $vcvars); $i++) {
        Start-Sleep -Seconds 15
        if ($i % 8 -eq 7) { Write-Host ('  ... {0} min' -f [int]((($i + 1) * 15) / 60)) }
    }

    if (-not (Test-Path $vcvars)) {
        throw 'Der Compiler ist nach 40 Minuten nicht aufgetaucht. Bitte den Visual Studio Installer oeffnen und die C++-Workload von Hand setzen.'
    }
    Write-Host '  C++-Workload installiert.' -ForegroundColor Green
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
