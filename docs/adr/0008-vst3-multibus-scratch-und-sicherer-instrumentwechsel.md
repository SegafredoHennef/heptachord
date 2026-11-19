# VST3-Multi-Bus-Scratchpuffer und sicherer Instrumentwechsel

Status: accepted

Mehrere VST3-Instrumente melden mehr Ausgangs- (und teils Eingangs-)Kanaele/Busse, als der Standalone-Host bereitstellt (Stereo). JUCE bildet aktive Busse fortlaufend auf Buffer-Kanaele ab; wird nur ein 2-Kanal-Puffer uebergeben oder werden Zusatzbusse deaktiviert, greifen manche Plugins trotzdem auf alle von ihnen erwarteten Kanalpuffer zu. Das fuehrte zu pluginseitigen Zugriffen ausserhalb des Puffers bzw. auf ungueltige Zeiger (u. a. Osirus, OsTIrus, Vavra, Xenia, NodalRed2x, FabFilter Pro-Q 4).

Zusaetzlich konnte beim Instrumentwechsel eine Race entstehen: Ein neu geladenes Plugin wurde veroeffentlicht, bevor es fuer die aktuelle Geraete-Rate vorbereitet (und sein interner/Scratch-Zustand dimensioniert) war, waehrend der Audio-Thread bereits `processBlock` aufrief. Auch das Zerstoeren des alten Instruments waehrend laufender Verarbeitung war moeglich.

## Entscheidung

- **Vollstaendige Kanalpuffer fuer das Plugin:** Vor `processBlock` wird bei Bedarf in einen Scratch-Puffer mit `max(Host-Kanaele, Plugin-Eingaenge, Plugin-Ausgaenge)` Kanaelen verarbeitet. Es wird nur eine View ueber die tatsaechlich anliegende Blockgroesse verwendet (keine groessere Allokation als Sample-Anzahl melden). Stereo 0/1 wird danach in den Host-Puffer zurueckkopiert.
- **Buslayout konservativ:** Der Haupt-Ausgang wird nur dann explizit auf Stereo gesetzt, wenn er deaktiviert/leer ist; weitere Busse bleiben in ihrem Default-Zustand. `setBusesLayout` wird nur bei echter Aenderung angewendet.
- **Sicherer Instrumentwechsel:** Das neue Instrument wird mit der aktuellen Sample-Rate vorbereitet, bevor der Instrumentzeiger fuer den Audio-Thread sichtbar wird. Zugriff/Tausch des Instruments sind ueber einen Mutex synchronisiert; das alte Instrument wird erst nach dem Tausch zurueckgesetzt/geschlossen/zerstoert.
- **Prepare-Disziplin:** `prepareToPlay` wird nicht unnoetig mit identischen Parametern wiederholt; `load` uebernimmt die bekannte Geraete-Rate, um doppelte Reinitialisierung zu vermeiden.

## Konsequenzen

- Multi-Bus-Instrumente erhalten gueltige Puffer fuer alle erwarteten Kanaele; die zuvor reproduzierbaren Host-seitigen Crashes der genannten Plugins sind im Auto-Test (Laden, Editor oeffnen/schliessen/erneut oeffnen, Wechsel, Noten) behoben.
- Geringer Mehraufwand durch Kopieren/Scratch-Puffer nur bei Plugins mit mehr Kanaelen als der Host-Puffer; der Mutex serialisiert Tausch und Verarbeitung, Wechsel sind selten.
- Plugins, die unabhaengig vom Host in ihrem eigenen Code fehlerhaft sind, koennen dadurch nicht korrigiert werden (siehe ADR 0009).
