# Auto-Test fuer alle VST3-Plugins: laedt jedes Plugin in Heptachord
# (HEPTACHORD_AUTO_VST), testet Editor, Instrument-Wechsel und Sound.
# Aufruf: powershell -File tools\test_all_vsts.ps1

$exe = 'C:\opencode-projekte\heptachord-v014\build\ninja\heptachord_artefacts\Release\Standalone\Heptachord.exe'
$log = 'C:\Users\zichosch.thomas\AppData\Local\Temp\opencode\dbg_audio.txt'
$out = 'C:\Users\zichosch.thomas\AppData\Local\Temp\opencode\vst_test_results.txt'

$vstRoot = 'C:\Program Files\Common Files\VST3'
$plugins = @(
    "$vstRoot\Nexus.vst3",
    "$vstRoot\OB-Xd 3.vst3",
    "$vstRoot\OB-Xd.vst3",
    "$vstRoot\Dexed.vst3",
    "$vstRoot\DX7 V.vst3",
    "$vstRoot\Diva(x64).vst3",
    "$vstRoot\Analog Lab 3.vst3",
    "$vstRoot\SongEngine_x64.vst3",
    "$vstRoot\Osirus.vst3",
    "$vstRoot\OsTIrus.vst3",
    "$vstRoot\Vavra.vst3",
    "$vstRoot\Xenia.vst3",
    "$vstRoot\JE8086.vst3",
    "$vstRoot\fantamidi.vst3",
    "$vstRoot\noteGRABBER.vst3",
    "$vstRoot\Plugin Buddy.vst3",
    "$vstRoot\NodalRed2x.vst3",
    "$vstRoot\Reason Rack Plugin.vst3",
    "$vstRoot\LANDR Samples.vst3",
    "$vstRoot\OsirusFX.vst3",
    "$vstRoot\OsTIrusFX.vst3",
    "$vstRoot\VavraFX.vst3",
    "$vstRoot\XeniaFX.vst3",
    "$vstRoot\Fracture.vst3",
    "$vstRoot\ValhallaSupermassive.vst3",
    "$vstRoot\Saturation Knob.vst3",
    "$vstRoot\Wider.vst3",
    "$vstRoot\Auburn Sounds Inner Pitch 2.vst3",
    "$vstRoot\LANDR Mastering Standard.vst3",
    "$vstRoot\LANDR Stems.vst3",
    "$vstRoot\Surge Synth Team\Surge XT.vst3",
    "$vstRoot\Xfer\Serum.vst3",
    "$vstRoot\u-he\ACE(x64).vst3",
    "$vstRoot\u-he\Bazille(x64).vst3",
    "$vstRoot\u-he\Hive.vst3",
    "$vstRoot\u-he\Repro-1(x64).vst3",
    "$vstRoot\u-he\Zebra2(x64).vst3",
    "$vstRoot\fury800_2_4_2\fury800.vst3",
    "$vstRoot\FabFilter\FabFilter Pro-Q 4.vst3"
)

$lines = @()
"VST-Test gestartet: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" | Set-Content $out

foreach ($vst in $plugins) {
    $name = Split-Path $vst -Leaf
    if (-not (Test-Path -LiteralPath $vst)) {
        $line = "$name | NOT_FOUND"
        $lines += $line; $line | Add-Content $out
        continue
    }

    Remove-Item $log -ErrorAction SilentlyContinue
    $env:HEPTACHORD_AUTO_VST = $vst

    $p = Start-Process -FilePath $exe -PassThru
    $exited = $false; $code = $null
    for ($i = 0; $i -lt 90; $i++) {
        Start-Sleep -Milliseconds 500
        $p.Refresh()
        if ($p.HasExited) { $exited = $true; $code = $p.ExitCode; break }
    }

    if (-not $exited) {
        Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue
        $line = "$name | HANG (45s Timeout)"
        $lines += $line; $line | Add-Content $out
        continue
    }

    $txt = Get-Content $log -Raw -ErrorAction SilentlyContinue
    if (-not $txt -or $txt -notmatch 'AUTO: loaded') {
        $line = "$name | LOAD_FAIL exit=$code"
        $lines += $line; $line | Add-Content $out
        continue
    }

    $editor  = if ($txt -match 'editor open2=1') { 'EDITOR_OK' }  else { 'EDITOR_FAIL' }
    $switch  = if ($txt -match 'switched to internal' -and $txt -match 'TESTSYNTH peak') { 'SWITCH_OK' } else { 'SWITCH_FAIL' }
    $back    = if ($txt -match 'switched back' -and $txt -match 'name=') { 'BACK_OK' } else { 'BACK_FAIL' }

    $peaks = [regex]::Matches($txt, 'peakAfter=([0-9.eE+-]+)') | ForEach-Object { [double]$_.Groups[1].Value }
    $maxPeak = 0.0
    foreach ($v in $peaks) { if ($v -gt $maxPeak) { $maxPeak = $v } }
    $sound = if ($maxPeak -gt 0.001) { 'SOUND=' + [math]::Round($maxPeak, 3) } else { 'NO_SOUND' }

    $done = if ($txt -match 'AUTO: done') { 'DONE' } else { 'NO_DONE' }

    $line = "$name | $editor | $switch | $back | $sound | exit=$code | $done"
    $lines += $line; $line | Add-Content $out
}

"" | Add-Content $out
"ENDE $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" | Add-Content $out
Write-Output ($lines -join "`n")
