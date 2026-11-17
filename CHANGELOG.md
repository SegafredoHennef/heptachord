# Changelog

Die Detailentscheidungen stehen im Log `docs/entscheidungen.md`.

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
