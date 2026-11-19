# Changelog

Die Detailentscheidungen stehen im Log `docs/entscheidungen.md`.

## v0.1.7

Stabiler Stand auf v0.1.4-Basis; die zurückgezogenen Linien v0.1.5/v0.1.6
werden damit abgelöst. Der Branch `release/v0.1.7-stable` enthält diesen Stand.

- VST3-Stabilität (ADR 0008): Scratch-Puffer mit der vollen Plugin-Kanalzahl
  für Multi-Bus-Instrumente, Buslayout nur bei echter Änderung, Instrument-
  Wechsel mit `prepare()` vor Veröffentlichung und Mutex-Schutz. Osirus,
  OsTIrus, Vavra, Xenia, NodalRed2x und FabFilter Pro-Q 4 laufen damit im
  vollen Auto-Test (Laden, Editor öffnen/schließen/neu öffnen, Wechsel, Noten).
- `Reason Rack Plugin.vst3` wird wegen eines pluginseitigen Absturzes blockiert
  (ADR 0009): klare Meldung statt Application Crash.
- Sechs Richtungsmodi (Vorwärts, Rückwärts, Pendel I/II, Brownian, Zufall),
  gelbe Step-Markierung, `currentStepIndex`-Fix.
- NOTEN-Export: MIDI + Text mit konsistenter Step-Spalte; ein Speichern-Dialog
  für beide Dateien (Entscheidung 88).
- Aufnahme über mehrere Zyklen (ADR 0010): slotbasierte Zykluszählung, Zielwahl
  unbegrenzt/1/2/4/8 mit Auto-Stop, Live-Anzeige `Zyklus n · Step m/8`,
  SPEICHERN für die gesamte Aufnahme (erste Note bei Tick 0, Abschluss-Offs),
  PLAY nur die Aufnahme ohne Engine-Noten.
- INSTRUMENT-Panel im Global-Bereich (aus v0.1.6 portiert); das VST-Editor-
  Fenster merkt Größe/Position, der EDITOR-Knopf zeigt den Offen-Zustand.
- GUI-Zoom in fünf Stufen (100/125/150/175/200 %) über einen eigenen
  Inhalts-Container: Layout bleibt konstant, Editor-Fenster wächst linear.
- Alle vier Stimmen (Thru, H1–H3) wieder standardmäßig aktiv; geladene
  VST-Instrumente erhalten MIDI auf Kanal 1–4 (vorerst nur THRU war hörbar).
- Editor-Fenster: Position nur bei voller Sichtbarkeit gemerkt; zu große
  Fenster (z. B. Surge XT) werden mit erreichbarer Titelzeile platziert.
- VST3-Ladedialog startet im Standardordner `Common Files\VST3`.
- Einheitliche Dateinamen `Heptachord_<Zeitstempel>_<Grundton>_<Skala>_<BPM>`
  (Aufnahme mit `Aufnahme_` im Präfix); zweiter Drag-Knopf `DRAG REC` für die
  gesamte Aufnahme. Das VST3-Plugin wird nicht per Drag übergeben (Doku).
- Prüfung: 150 Testfälle / 15757 Assertions, `ctest --preset ninja` 100 %,
  VST-Auto-Tests der sechs zuvor crashenden Instrumente mit `exit=0`.

ZIP `Heptachord-0.1.7-win.zip` (5,4 MB), SHA-256:
`1A6731BF9F5FC61513B621A6AF0C925FFB4212C33C38BD35F579ECBC055A9556`

## v0.1.4

- WAV-Export im Standalone: einen Zyklus durch den Test-Synth rendern und
  als 16-Bit-WAV speichern (src/plugin/AudioExport.cpp, M3). "WAV"-Knopf
  in der Steps-Titelzeile, standalone-only. Dateiname
  `Heptachord_<Grundton>_<Skala>_<Rate>.wav` (ASCII).
- Prüfung: 146 Testfälle / 15505 Assertions, `ctest --preset ninja` 100 %,
  pluginval `--strictness-level 5` SUCCESS.

ZIP `Heptachord-0.1.4-win.zip` (5,2 MB), SHA-256:
`64EE909D6A0E0AEF876F4E23627A45C8547AE9FDD176231014CA34290439D9D3`

## v0.1.3

- MIDI-Export im Standalone: ein Zyklus als Standard-MIDI Format 1 (PPQ 480,
  4 Spuren, Tempo-Meta), deterministisch mit festem Seed 1 — identische
  Parameter erzeugen eine bitgleiche Datei (Entscheidung 84). Letzte
  Eingangsnote, sonst C4 (MIDI 60). Gate-Enden und Hold-Ketten werden
  als Note-Offs mitgeschrieben.
- Drag & Drop: „DRAG"-Knopf in der Steps-Titelzeile zieht eine temporäre
  `.mid`-Datei in die DAW (Entscheidung 85). „EXPORT" öffnet den
  Speichern-Dialog; beide Knöpfe nur im Standalone (Entscheidung 86).
- Dateiname: `Heptachord_<Grundton>_<Skala>_<Rate>.mid` (ASCII).
- Prüfung: 143 Testfälle / 15493 Assertions, `ctest --preset ninja` 100 %,
  pluginval `--strictness-level 5` SUCCESS.

ZIP `Heptachord-0.1.3-win.zip` (4,8 MB), SHA-256:
`EF0265D657E2D708320FEAB20A4A063F9B42A95B2197976C93425188F0A6B993`

## v0.1.2

- 96 Factory-Presets: alle 12 Grundtöne × 8 Skalen (statt acht, je eins
  pro Skala). Je Skala bleibt das Charakter-Muster erhalten, nur der
  Grundton transponiert; Dateinamen sind nummerierte stabile chromatische
  IDs (`heptachord_89_h_dur.heptachord`), der Selektor lädt in stabiler
  Reihenfolge (Entscheidung 82).
- Bugfix im Preset-Generator: Overrides standen als tote
  Unterstrich-Schlüssel (`s3_mode`) in den Dateien und wurden beim Laden
  still ignoriert — die Charakter-Pattern der Factory-Presets waren nie
  hörbar. Jetzt echte Parameter-IDs (`s3.mode`); `C Dur` bleibt
  byte-identisch, die anderen C-Dateien bekommen das dokumentierte Muster
  erstmals wirklich (Entscheidung 82).
- Bezeichnungsmodus: Combo „Bezeichnungen" in der Kopfzeile — Deutsch
  (C, Cis, …, Ais, H; Dur, Dorisch, …), International (b) (C, Db, …, Bb, B)
  oder International (#) (C, C#, …, A#, B). Betrifft nur die Anzeige
  (root/scale-Combos, Preset-Anzeigenamen); Presets speichern Indizes, ein
  Preset-Wechsel setzt den Modus nicht zurück. Grundton-Choices enden
  jetzt auf „Ais"/„H" (statt „B") (Entscheidung 83).
- Literaturhinweise zur Musiktheorie: `docs/literatur.md` (verifizierte
  freie PDFs, Zuordnung zu den Heptachord-Begriffen).

Prüfung: 129 Testfälle / 14550 Assertions, `ctest --preset ninja` 100 %,
pluginval `--strictness-level 5` SUCCESS.

ZIP `Heptachord-0.1.2-win.zip` (4,7 MB), SHA-256:
`75C9FA57913A769E250871331E9E6D15469ED7BFBEBAA7596177336F202B9A05`

## v0.1.1

- Fix: Die QWERTZ-Mini-Klaviatur im Standalone blieb bei echter Tastatur
  stumm — es wurden nur Zeichencodes (`'a'..'z'`) gemappt, echte Tasten
  liefern aber VK-Codes (`'A'..'Z'`); nur die Leertaste wirkte.
  `mapKey` normalisiert die Codes jetzt; Regressionstest ergänzt
  (Entscheidung 81).
- MIDI-Eingang, Presets, Skala und Engine waren nicht betroffen.

Prüfung: 120 Testfälle / 3054 Assertions, `ctest --preset ninja` 100 %,
pluginval `--strictness-level 5` SUCCESS.

ZIP `Heptachord-0.1.1-win.zip` (4,6 MB), SHA-256:
`4637B4D4E118520C6D653C276A0109ABBB0C1B4AE13C30C5D2A37FB9DADF0710`

## v0.1.0

- Signalweg: 12 Eingangstöne × 8 Skalen × 4 Stimmen, diatonische Intervalle,
  Snap auf den nächsten Skalenton, Root-relative Rastung (M0–M2).
- Step-Engine: 17 Raten, Swing, Slack, Ratchet, Probability, PingPong, Rest,
  flankengetriggertes `reset` (M1–M3).
- Editor: einseitiges Layout 1000×664, eigenes dunkles Look-and-Feel, alle
  115 Parameter aus `parameters.toml` (M3).
- Presets: JSON `.heptachord` (schemaVersion 1), acht Factory-Presets (je eins
  pro Skala), Selektor in der Kopfzeile (M5).
- QWERTZ-Mini-Klaviatur im Standalone: A–K = C–C, Y/Z Oktave runter,
  X hoch, Leertaste Transport (M5).
- MIDI-Clock: im Standalone folgt der Transport dem ersten MIDI-Eingang
  (FA/FC/FB, 24 Clocks pro Viertel, Tempo 30–300 BPM, 2-s-Timeout mit
  Fallback auf 120 BPM); das VST3 nutzt den Host-PlayHead (M6).

Prüfung: 119 Testfälle / 3035 Assertions, `ctest --preset ninja` 100 %,
pluginval `--strictness-level 5` SUCCESS.

ZIP `Heptachord-0.1.0-win.zip` (4,6 MB), SHA-256:
`FA7B41E172A09BFCDE124AFFC38AEBCD47BDEB1E4DB6F2F6A79C6EEEBAD6994F`
