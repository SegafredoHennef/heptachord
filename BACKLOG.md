# Backlog — Heptachord

Offene Aufgaben und Ideen. Entscheidungen stehen in `docs/entscheidungen.md`.

## Offen

- [ ] GUI-Zoom (100/125/150/175/200 %) aus v0.1.6 auf v0.1.4-Basis portieren
- [ ] INSTRUMENT-Panel im Global-Bereich (aus v0.1.6) portieren
- [ ] "Skala zwei mal auswaehlbar" — klaeren, welche ComboBox entfernt werden soll
- [ ] Richtung aus oberster Zeile entfernen (Position klaeren)
- [ ] main-Branch bereinigen (v0.1.5/v0.1.6-Features die crashen entfernen)
- [x] NOTEN-Export (MIDI + Text): ein Speichern-Dialog fuer beide Dateien,
      Text mit konsistenter Step-Spalte (1-basiert, Off gehoert zum On), Tests in test_midirender.cpp
- [ ] VITAL-Editor-Fenster: Groesse/Position merken, ggf. Schliessen-Button-Feedback
- [ ] VST-Durchprobierung (tools/test_all_vsts.ps1): verbleibende problemfaellige Plugins klaeren —
  HANG "Saturation Knob" (Editor/Lizenz-Dialog?), LOAD_FAIL "LANDR Samples",
  kein Sound trotz Instrument: DX7 V, Analog Lab 3, SongEngine, OB-Xd 3 (evtl. kein Default-Patch
  oder zu lange Ladezeit — Auto-Test-Timing pruefen)
- [ ] Effekt-VSTs (ValhallaSupermassive, Wider, Inner Pitch, LANDR Mastering/Stems, Fracture)
  bringen als Instrument keinen Sound — erwartet; nur relevant, wenn Effekt-Support gewuenscht
- [ ] Reason Rack Plugin.vst3: Blocklist entfernen, sobald ein Hersteller-Update den pluginseitigen
  Absturz (AV in processBlock) nachweislich behebt (ADR 0009)

## Erledigt

- [x] VST3 Multi-Bus-Scratchpuffer + sicherer Instrumentwechsel (Mutex/prepare vor Veroeffentlichung):
      Osirus, OsTIrus, Vavra, Xenia, NodalRed2x, FabFilter Pro-Q 4 im Auto-Test stabil
      (Laden, Editor open/close/reopen, Wechsel, Noten), vgl. ADR 0008
- [x] Reason Rack Plugin.vst3 als inkompatible Version blockiert (klare Meldung, kein Load/Crash),
      pluginseitiger Absturz in processBlock, vgl. ADR 0009
- [x] Debug-Logging (dbg_audio.txt) auf Diagnose/Auto-Test begrenzt (opt-in per
      HEPTACHORD_VST_DEBUG/HEPTACHORD_AUTO_VST), kein Default-Write im Normalbetrieb
- [x] VST-Durchprobierung aller 39 VST3 im System (tools/test_all_vsts.ps1) — 15 klingen,
      7 pluginseitige Crashes, 1 HANG, 1 LOAD_FAIL, Rest sauber ohne Sound (Effekte/MIDI-Tools erwartet)
- [x] Surge XT nachgetestet (Pfad "Surge Synth Team") — klingt (peak 0.337), alles OK
- [x] Instrument-Wechsel VST <-> interner Synth (ComboBox) + VST-Editor zweites Mal oeffnen
- [x] 5 Welthit-MIDIs + README/DOCX in midi-progressions-20251006/5-Welthits/
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