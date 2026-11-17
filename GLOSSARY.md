# Heptachord

Heptachord ist ein stepweiser diatonischer Harmonizer: Aus einer gehaltenen
Eingangsnote erzeugt er vier MIDI-Stimmen, die im Raster eines Acht-Step-
Sequencers laufen. Die Begriffe hier beschreiben die musikalische Domäne,
nicht die Implementierung.

## Language

**Step**:
Einer von acht gleich langen Abschnitten eines Zyklus, an deren Grenze die
Stimmen neu getriggert werden.
_Avoid_: Takt, Beat, Schlag

**Voice**:
Eine der vier MIDI-Ausgabestimmen: Thru, H1, H2, H3.
_Avoid_: Kanal, Part, Spur

**Thru**:
Die Stimme, die die Eingangsnote unverändert ausgibt.
_Avoid_: Input, Dry, Original

**Harmoniestimme**:
Eine der drei Stimmen H1–H3, deren Tonhöhe aus einem diatonischen Abstand zur
Eingangsnote folgt.
_Avoid_: H-Voice, Harmony Part, Begleitung

**Eingangsnote**:
Die Note, aus der alle vier Stimmen abgeleitet werden — die zuletzt gespielte
Note.
_Avoid_: Source, Ausgangsnote, Trigger-Note

**Grundton**:
Der Bezugston der gewählten Skala (C…H in deutscher Konvention; die Anzeige
folgt dem Bezeichnungsmodus).
_Avoid_: Root Note, Key, Tonika

**Skala**:
Eine der acht Stufentabellen (Dur, Moll, Harmonisch Moll, Melodisch Moll,
Dorisch, Phrygisch, Lydisch, Mixolydisch), die festlegt, welche Halbtonstufen
innerhalb einer Oktave skaleneigen sind.
_Avoid_: Mode, Tonart, Temperament

**Bezeichnungsmodus**:
Die Anzeige-Konvention für Noten- und Skalennamen: Deutsch (C, Cis, …, Ais, H;
Dur, Dorisch, …), International (b) (C, Db, …, Bb, B) oder International (#)
(C, C#, …, A#, B). Betrifft nur die Anzeige — Presets speichern Indizes, und
ein Preset-Wechsel setzt den Modus nicht zurück.
_Avoid_: Sprache, Lokalisierung, Enharmonik

**Skalengrad**:
Die Position eines Tons innerhalb der Skala, gezählt in Skalenschritten statt
in Halbtönen.
_Avoid_: Intervall, Stufe, Index

**Gradabstand**:
Der diatonische Abstand einer Harmoniestimme zur Eingangsnote, ausgedrückt in
Skalenschritten und vorzeichenbehaftet.
_Avoid_: Intervall, Transposition, Offset

**Rastung**:
Das Zurückführen eines chromatischen Tons auf den nächstgelegenen Skalenton.
_Avoid_: Quantisierung, Snap-to-Grid

**Rate**:
Die Länge eines Steps als musikalischer Teiler (1/16 … 8), nicht als freie
Frequenz.
_Avoid_: Tempo, Speed, Clock

**Step-Modus**:
Das Verhalten der Stimmen innerhalb eines Steps: Play, Hold, Rest oder Ratchet.
_Avoid_: Pattern, Trigger-Art

**Ratchet**:
Mehrere gleichmäßige Trigger innerhalb eines Steps bei konstanter Tonhöhe.
_Avoid_: Roll, Repeat, Stutter

**Swing**:
Die Verzögerung der ungeraden Steps um einen Anteil der Step-Länge.
_Avoid_: Shuffle, Groove

**Slack**:
Ein zusätzlicher Timing-Versatz in Millisekunden, unabhängig vom Tempo.
_Avoid_: Humanize, Delay, Late

**Gate**:
Die Dauer, für die eine Stimme innerhalb eines Steps klingt, ausgedrückt
relativ zur Step-Länge.
_Avoid_: Notenlänge, Duration, Sustain

**Velocity-Faktor**:
Ein multiplikativer Faktor auf die Velocity der Eingangsnote für eine
bestimmte Stimme.
_Avoid_: Velocity, Anschlagstärke

**Latch**:
Das Festhalten der zuletzt gespielten Eingangsnote, nachdem die Taste
losgelassen wurde.
_Avoid_: Hold, Sustain, Freeze

**Aktive Steps**:
Die Teilmenge der acht Steps, die am Zyklus teilnimmt.
_Avoid_: Pattern-Länge, Maske

**Output-Modus**:
Die Art, wie die vier Stimmen auf MIDI-Kanäle verteilt werden: `Multi` (eine
Stimme pro Kanal) oder `Single` (alle Stimmen auf einem Kanal).
_Avoid_: Routing, Kanalmodus, Split

**Preset**:
Ein vollständiger, benannter Parametersatz, gespeichert als
`.heptachord`-Datei (JSON, `schemaVersion` 1). Choice-Parameter liegen als
Index, Bool als 0/1, der Trigger `reset` ist ausgenommen.
_Avoid_: Patch, Sound, Bank
