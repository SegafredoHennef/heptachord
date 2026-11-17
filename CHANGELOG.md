# Changelog

Die Detailentscheidungen stehen im Log `docs/entscheidungen.md`.

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
