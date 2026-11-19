# Reason Rack Plugin als inkompatibles VST3 blockieren

Status: accepted

Das "Reason Rack Plugin.vst3" (Reason Studios, Version 14.0.0.8573) stuerzt im Standalone reproduzierbar im eigenen `processBlock` ab: Access Violation (0xC0000005) bei RVA `0x1701850` (Schreibschleife ueber Kanalpuffer). Der Absturz tritt wenige Sekunden nach Start/Initialisierung auf — unabhaengig von Editor-Anzeige, Instrumentwechsel, Note-Events, Sample-Rate (44,1/48 kHz), Blockgroesse oder Bus-/Prepare-Varianten. Beim Beenden kann zusaetzlich Heap-Korruption auftreten.

Die Analyse (Log-Zeitpunkt, WER/Crashlogs, Disassembly der betroffenen Stelle) weist auf einen Defekt im Plugin selbst hin. Der Host kann den Binaercode eines Drittanbieter-Plugins nicht korrigieren; ein Laden wuerde die Anwendung destabilisieren.

## Entscheidung

- Das Plugin wird anhand des Dateinamens (`Reason Rack Plugin.vst3`) **vor** der Instanziierung/Ladung erkannt und nicht geladen.
- Beim manuellen Laden erhaelt der Nutzer eine klare Begruendung (pluginseitiger Absturz, Inkompatibilitaet dieser Version, Hinweis auf Hersteller-Update).
- Im Auto-Test wird der Fall als `AUTO: blocked ...` protokolliert und der Prozess beendet sich sauber (kein Crash).

## Konsequenzen

- Heptachord bleibt stabil, auch wenn das Plugin auf dem System installiert ist; das Plugin ist in dieser Version innerhalb von Heptachord nicht nutzbar.
- Die Sperre ist gezielt und version-/dateinamensbezogen formuliert; sie muss angepasst oder entfernt werden, wenn ein Hersteller-Update das Problem nachweislich behebt.
- Weitere Host-seitige Workarounds ohne Spezifikation des Plugins waeren spekulativ und werden nicht umgesetzt.
