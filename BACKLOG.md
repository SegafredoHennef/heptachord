# Backlog — Heptachord

Offene Aufgaben und Ideen. Entscheidungen stehen in `docs/entscheidungen.md`.

## Offen

- [ ] GUI-Zoom (100/125/150/175/200 %) aus v0.1.6 auf v0.1.4-Basis portieren
- [ ] INSTRUMENT-Panel im Global-Bereich (aus v0.1.6) portieren
- [ ] "Skala zwei mal auswaehlbar" — klaeren, welche ComboBox entfernt werden soll
- [ ] Richtung aus oberster Zeile entfernen (Position klaeren)
- [ ] main-Branch bereinigen (v0.1.5/v0.1.6-Features die crashen entfernen)
- [ ] Noten-Export testen (MIDI + Text/CSV)
- [ ] Debug-Logging (dbg_audio.txt) vor Release entfernen — InstrumentHost.cpp + PluginEditor.cpp (Auto-Test-Log)
- [ ] VITAL-Editor-Fenster: Groesse/Position merken, ggf. Schliessen-Button-Feedback

## Erledigt

- [x] VST3-Loading mit VITAL getestet — klingt (peakAfter ~0.53, Auto-Test)
- [x] VST3 Ausgabe-Bus-Fix (outCh=0, nicht kDefaultActive) + echte Device-Rate beim Instrument-Wechsel
- [x] Auto-Test-Modus HEPTACHORD_AUTO_VST (laedt VST3, spielt Note, beendet sich)
- [x] VITAL Editor als eigenes Fenster mit Titelleiste/Schliessen-Button
- [x] VITAL MIDI-Routing (processBlock mit MIDI-Eingabe + prepareToPlay)
- [x] Gelbe Step-Markierung (v0.1.7)
- [x] 6 Richtungsmodi (Pendel I/II, Brownian)
- [x] VST3-Instrument-Loading im Standalone
- [x] Tempo-Parameter (30-300 BPM)
- [x] currentStepIndex-Crash-Fix
- [x] VST3-Loading-Fix (addDefaultFormatsToManager)
- [x] Export-Buttons vergroessert
- [x] Richtung/Rate/Output ComboBoxes befuellt
- [x] Fokus auf Step 1 beim Start
- [x] h1/h2/h3 standardmaessig ausgeschaltet (nur Standalone)