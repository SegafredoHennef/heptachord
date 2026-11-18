# Heptachord

Stepweiser diatonischer Harmonizer: Aus einer gehaltenen Eingangsnote erzeugt
Heptachord vier MIDI-Stimmen (Thru + H1–H3), die im Raster eines Acht-Step-
Sequencers laufen. Jede Harmoniestimme folgt einem diatonischen Gradabstand,
nie einer festen Halbtonzahl — dadurch bleibt jede Stimme in der Skala.

Heptachord ist ein **Instrument mit stillem Audio-Ausgang**: es verarbeitet kein
Audio, sondern gibt Noten auf MIDI-Kanälen aus (ADR 0001). Die Standalone-
Variante bringt einen einfachen Test-Synth mit.

**Status: v0.1.4.** Signalweg (M2b), Oberfläche (M3), Presets (M5)
und MIDI-Clock sind fertig: alle 117 Parameter liegen auf einer Seite (globale
Leiste, Stimmen, acht Step-Spalten), das Erscheinungsbild ist ein eigenes
dunkles Look-and-Feel, 96 Factory-Presets (12 Grundtöne × 8 Skalen) laden
aus der Kopfzeile, und der Bezeichnungsmodus schaltet die Noten- und
Skalennamen zwischen Deutsch, International (b) und International (#) um.
Der Standalone exportiert einen Zyklus als MIDI-Datei (deterministisch,
Seed 1) oder zieht ihn per Drag & Drop in eine DAW, sowie als WAV (Test-Synth).
Der Standalone folgt im Play-Modus dem ersten MIDI-Eingang als Clock-Master
(FA/FC/FB, 30–300 BPM, 2 s Timeout → Fallback auf 120 BPM); ohne Clock läuft
der manuelle Transport wie bisher.

## Struktur

| Pfad | Inhalt |
|---|---|
| `src/core` | Domänenlogik, **ohne JUCE** (Skalen, Step-Engine, Notenlogik, Parameterabbildung) |
| `src/plugin` | JUCE-Wrapper, Parameter-Bindung, MIDI-Brücke, Test-Synth |
| `tests` | Catch2-Tests, Golden Files, Referenzdaten |
| `tools` | Generatoren (Parameter-Tabelle, Python-Referenz) |
| `docs` | Spezifikation, ADRs, Entscheidungs-Log |
| `parameters.toml` | Parameter-Tabelle als Single Source of Truth |

## Parameter

`parameters.toml` ist die Quelle. `src/core/ParameterTable.h`,
`docs/02-parameter.md` und `tests/parameter_ids.txt` werden daraus erzeugt:

```
python tools/gen_params.py          # schreiben
python tools/gen_params.py --check  # CI-Prüfung auf Drift
```

Parameter-IDs sind eingefroren (ADR 0003, `docs/entscheidungen.md` Nr. 40).

## Benutzung

**In der DAW (VST3):** als Instrument laden, eine Note halten und den
MIDI-Ausgang des Plugins auf ein Ziel-Instrument routen (Live: „MIDI To",
Reaper: Routing-Matrix, Bitwig: Note-Effekt-Kette, Cubase: MIDI-Send). Der
Audio-Ausgang von Heptachord ist absichtlich still.

**Standalone:** Note über die eingebaute Mini-Klaviatur spielen (A-K = C bis C
einer Oktave, Y oder Z = Oktave runter, X = Oktave rauf, Leertaste =
Transport), alternativ über MIDI-Eingabe. Leertaste oder der Knopf startet und
stoppt den internen Transport (120 BPM).

## Download

Aktuelle stabile Version: **v0.1.5** (`Heptachord-0.1.5-win.zip`, SHA-256 steht im
`CHANGELOG.md`).

**Hinweis:** v0.1.6 hatte einen Standalone-Executable-Absturz (siehe CHANGELOG.md).
Alle Features (Step-Highlight, INSTRUMENT-Panel, GUI-Zoom, kein Reset) sind in
v0.1.5 vollständig implementiert und der Standalone startet fehlerfrei.

- `VST3/Heptachord.vst3` — in den VST3-Ordner der DAW kopieren
  (z. B. `C:\Program Files\Common Files\VST3`), dann einmal scannen lassen.
- `Standalone/Heptachord.exe` — direkt startbar, keine Extra-DLLs nötig.
- `presets/` — die 96 Factory-Presets; der Ordner bleibt neben den
  Programmdateien liegen und wird von beiden Varianten gefunden.

**Keine Signierung** (Entscheidung 48): Windows zeigt beim ersten Start des
Standalone den SmartScreen-Hinweis „Windows hat deine Datei geschützt" —
„Mehr Informationen" und dann „Trotzdem ausführen" wählen. Das ist der
übliche Hinweis für unsignierte, öffentlich verteilte Programme.

## Bauen (in M0 verifiziert)

Voraussetzungen: Windows 10/11, Visual Studio 2026 Build Tools mit
C++-Workload (MSVC v143), CMake ≥ 3.28, Ninja, Python ≥ 3.11, Git.

JUCE hängt als flaches Submodule am Tag `9.0.5`:

```
git submodule update --init --depth 1
cmake --preset vs           # oder: --preset ninja (aus der Developer Shell)
cmake --build --preset vs-release
ctest --preset vs-release
```

`tools/setup/windows-toolchain.ps1` richtet die Toolchain ein (Adminrechte
nötig, weil die C++-Workload maschinenweit installiert wird).

## Doku

- `docs/02-parameter.md` — Parametertabelle (generiert)
- `docs/03-spezifikation.md` — Verhalten
- `docs/entscheidungen.md` — Entscheidungs-Log der Design-Runden
- `docs/adr/` — harte Entscheidungen
- `GLOSSARY.md` — Begriffe der Domäne

## Herkunft

Heptachord ist funktional aus der Analyse des Reason-Rack-Extension
„AstroPath Stepwise Harmonizer" abgeleitet. Es wird **kein Code, keine Grafik,
keine Presets und kein Name** des Originals übernommen; alle Implementierungen
sind eigenständig.

## Lizenz

AGPL-3.0 (siehe `LICENSE`). Der Quellcode ist vollständig eigen; JUCE 9 steht
unter AGPLv3 (ADR 0002).
