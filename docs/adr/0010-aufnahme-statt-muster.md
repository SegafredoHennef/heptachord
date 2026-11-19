# Aufnahme statt Muster: Standalone-Take und Ein-Zyklus-Exporte

Status: accepted

Der Standalone konnte Noten ueber mehrere Zyklen aufnehmen, hatte aber keine
belastbare Positionsanzeige und keine reproduzierbare Aufnahmebegrenzung.
Schwerwiegender: Die Knoepfe EXPORT/DRAG/WAV/NOTEN lesen die Aufnahme gar
nicht, sondern rendern ueber `renderCycle()` genau einen Zyklus aus den
aktuellen Parametern. Eine mehrzykliche Aufnahme wurde beim „Speichern" auf
den ersten Zyklus gekuerzt. Der Recorder nutzte ausserdem eine
Start-Step-Heuristik (bei Zufall/PingPong unzuverlaessig), startete unquantisiert
trotz anders lautendem Tooltip, haengte an vorhandene Aufnahmen an und schrieb
MIDI mit fuehrender Stille und ohne Abschluss-Noten-Offs.

## Entscheidung

- **Aufnahme ist ein Take, kein Pattern:** eine Einheit ueber einen oder
  mehrere Zyklen, nur im RAM der Standalone-Sitzung. Presets und Plugin-State
  bleiben Parametersaetze. Ein neues REC ersetzt den vorherigen Take.
- **Zyklus = `activeCount` Step-Slots**, richtungsunabhaengig und identisch mit
  dem Export. Der Recorder zaehlt ueber `StepEngine::nextSlotIndex()`; es gibt
  kein Rueckkehr-zum-Start-Step-Kriterium mehr.
- **REC an der naechsten Step-Grenze**; der Transport startet bei Bedarf
  automatisch. Zielwahl `0 = unbegrenzt / 1 / 2 / 4 / 8` mit Auto-Stop an der
  Zyklusgrenze.
- **Live-Anzeige** `Zyklus n · Step m/8` (1-basiert, sichtbarer Slot S1–S8)
  direkt bei den Recorder-Knoepfen; bei Ziel bleibt die Anzeige auf dem
  erreichten Ziel stehen.
- **SPEICHERN** ist ein eigener, sichtbarer Knopf und schreibt die gesamte
  Aufnahme als SMF Format 1, PPQ 480, vier Spuren, erste Note bei Tick 0 und
  Note-Offs fuer noch klingende Stimmen. STOP speichert nicht mehr automatisch.
- **PLAY** spielt nur die Aufnahme (Engine-Noten werden verworfen), ab Tick 0.
- **EXPORT/DRAG/WAV/NOTEN bleiben Ein-Zyklus-Exporte** aus den Parametern
  (Entscheidungen 84/85/87/88); ihre Tooltips benennen das ausdruecklich.
  NOTEN-`.txt` behaelt die Step-Spalte (Entscheidung 88).

## Consequences

- Der reproduzierbare Datenverlust („nur die ersten Steps gespeichert") ist
  behoben: die Aufnahme wird ueber einen eigenen, sichtbaren Pfad vollstaendig
  gespeichert.
- Es gibt bewusst zwei Wege: parameterbasierter Ein-Zyklus-Export (EXPORT etc.)
  und aufgenommener Take (SPEICHERN). Die Tooltips halten sie auseinander.
- Die Aufnahme geht bei CLEAR oder Neustart verloren; ein Modell mit mehreren
  benannten Takes oder Pattern-Baenken ist eine spaetere, eigenstaendige
  Entscheidung.
- Die Zykluszahl ist eine Anzahl von Step-Slots, kein musikalischer Takt; das
  Glossar haelt Zyklus und Takt ausdruecklich auseinander.
