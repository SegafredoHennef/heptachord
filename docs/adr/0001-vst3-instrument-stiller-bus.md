# VST3-Instrument mit stillem Audio-Ausgang statt „MIDI-Effekt"

Status: accepted (ersetzt die ursprüngliche Annahme „VST3-MIDI-Effekt ohne Audio-Busse")

Das Original (AstroPath Stepwise Harmonizer) ist ein Reason-Player: es erzeugt
keine Audiodaten. Heptachord soll das als VST3 abbilden — aber VST3 kennt keine
MIDI-Effect-Kategorie, JUCE wertet `JucePlugin_IsMidiEffect` für VST3 nicht aus
(nur AU/AAX), und Ableton Live, Cubase und Studio One instanziieren Plugins
ohne Audio-Bus gar nicht. Deshalb wird das Plugin als **Instrument**
(`JucePlugin_IsSynth`, Kategorie `Instrument|Synth`) mit **stillem Stereo-
Ausgang** und MIDI-In/MIDI-Out gebaut; es verarbeitet weiterhin kein Audio und
leert die Puffer. Nur die Standalone-Variante ergänzt einen Test-Synth.

## Considered Options

- **Reiner MIDI-Effekt (keine Busse):** technisch nicht als VST3-Kategorie
  existent; in Live/Cubase/Studio One nicht ladbar.
- **Zwei Builds** (Note-FX für Bitwig/Reaper, Instrument für den Rest): doppelte
  Testmatrix, für v1 zu teuer — bleibt Option für M6.

## Consequences

- Jeder Host lädt Heptachord als Instrument; die Noten gehen über den MIDI-Ausgang
  auf eine zweite Spur (Routing je Host dokumentiert).
- Hosts, die MIDI-Kanäle beim internen Routing zusammenlegen (Live), verlieren
  die Kanal-Trennung der Stimmen — dafür gibt es einen Single-Channel-Modus.
- Es gibt keinen Audio-Renderpfad; Pegel-/Aliasing-Tests entfallen, alle Tests
  laufen über Noten-Events.
