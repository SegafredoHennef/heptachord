# Core ohne JUCE, vier Targets

Der musikalische Kern — Skalen, Grad→Halbton, Step-Engine, Noten- und
Kanalverwaltung — lebt in einer statischen Bibliothek `heptachord_core`, die
**kein JUCE** einbindet. Darüber liegen `heptachord_tests` (Catch2, linkt nur
den Core), `heptachord_plugin` (JUCE-Wrapper, VST3) und
`heptachord_standalone` (JUCE-Anwendung mit Test-Synth).

Ziel ist die Testbarkeit: Die 12×8×4-Abnahmetabelle und die Step-Timing-Tests
sollen als schnelle, host-freie Unit-Tests laufen, ohne dass ein JUCE-Header,
ein Message-Thread oder ein AudioDeviceManager im Weg steht. Zusätzlich
verhindert die Trennung, dass `juce::var`, `juce::String` oder APVTS-Typen in
die Domänenlogik sickern — der Core bleibt bei Standard-C++ und primitiven
Typen, die Serialisierung passiert ausschließlich im Plugin.

## Consequences

- Jede neue Domänenaufgabe braucht eine Schnittstelle, die ohne JUCE
  beschreibbar ist; das ist bei Zeit-/MIDI-Logik ohnehin der bessere Zuschnitt.
- Der Plugin-Wrapper ist dünn: Parameter aus APVTS in das Core-Struct kopieren,
  `processBlock` reicht Noten-Events durch, Core-Ausgabe in MIDI übersetzen.
- Die Standalone teilt sich den Core, hat aber einen eigenen Transport.
