# 02 — Parameter (v1)

Generiert aus `parameters.toml` (schemaVersion 1), nicht von Hand
bearbeiten. Die Tabelle ist die Quelle für `src/core/ParameterTable.h`, die
UI-Metadaten und das Preset-Schema (ADR 0003).

**ID-Schema**: `<gruppe>.<name>`, ASCII, kleingeschrieben. Globale Parameter
haben kein Präfix. Einzige erlaubte Indexform ist `s1…s8` und `h1…h3`.
IDs sind eingefroren; `tests/parameter_ids.txt` ist das Golden File.

**Preset-Kopf** (kein Parameter): `schemaVersion`, `name`, `author`, `notes`.

**Einheiten**: Velocity immer in Prozent (0…200 %, Default 100 %). Intern
multiplikativ, geklemmt auf 1…127.

**Summe**: 118 Parameter.

## Global

| ID | Label | Typ | Bereich | Default | Anmerkung |
|---|---|---|---|---|---|
| `root` | Grundton | choice | 0…11 (C / Cis / D / Dis / E / F / Fis / G / Gis / A / Ais / H) | C | Bezugston der Skala; Anzeige folgt names.mode |
| `scale` | Skala | choice | 0…7 (Dur / Moll / Harmonisch Moll / Melodisch Moll / Dorisch / Phrygisch / Lydisch / Mixolydisch) | Dur | Reihenfolge wie im Original |
| `rate` | Rate | choice | 0…16 (1/16 / 1/12 / 1/8 / 1/6 / 1/4 / 1/3 / 3/8 / 1/2 / 2/3 / 3/4 / 1 / 4/3 / 3/2 / 2 / 3 / 4 / 8) | 1/8 | 17 Werte, 1.0 = ein Takt (4/4) |
| `direction` | Richtung | choice | 0…3 (Vorwärts / Rückwärts / PingPong / Zufall) | Vorwärts | läuft nur über aktive Steps |
| `swing` | Swing | float | 0…75 % | 0 % | verzögert die ungeraden Steps |
| `slack` | Slack | float | -100…100 ms | 0 ms | globaler Versatz, auf weniger als eine Step-Länge geklemmt |
| `latch` | Latch | bool | an/aus | aus | hält die zuletzt gespielte Note |
| `output.mode` | Ausgabe | choice | 0…1 (Multi / Single) | Multi | Multi = eine Stimme pro Kanal, Single = alle auf thru.channel |
| `reset` | Reset | trigger | — | — | setzt am nächsten Step-Rand auf Step 1 |
| `names.mode` | Bezeichnungen | choice | 0…2 (Deutsch / International (b) / International (#)) | Deutsch | Anzeige-Konvention fuer Noten- und Skalennamen; wird beim Laden von Presets nicht ueberschrieben |
| `tempo` | Tempo | float | 30…300 BPM | 120 BPM | Standalone-Tempo in BPM; wird vom MIDI-Clock-Eingang überschrieben, solange aktiv |

## Thru

| ID | Label | Typ | Bereich | Default | Anmerkung |
|---|---|---|---|---|---|
| `thru.enable` | Aktiv | bool | an/aus | an |  |
| `thru.snap` | Thru mitrasten | bool | an/aus | aus | an = auch Thru wird auf die Skala gesnappt |
| `thru.velocity` | Velocity | float | 0…200 % | 100 % | Faktor auf die Eingabe-Velocity |
| `thru.channel` | Kanal | int | 1…16 | 1 | im Modus Single der Master-Kanal |

## Harmonien

| ID | Label | Typ | Bereich | Default | Anmerkung |
|---|---|---|---|---|---|
| `harm.velocity` | Velocity H1–H3 | float | 0…200 % | 100 % |  |
| `harm.spread` | Spread | float | 0…1 | 0.3 | spread_i = 1 − spread × (i−1)/2, i = H1…H3 |

## Stimmen

| ID | Label | Typ | Bereich | Default | Anmerkung |
|---|---|---|---|---|---|
| `h1.enable` | H1 · Aktiv | bool | an/aus | an |  |
| `h1.degree` | H1 · Grad | int | -14…14 | 2 | diatonische Skalenschritte |
| `h1.octave` | H1 · Oktave | int | -3…3 | 0 | chromatisch, nach degree |
| `h1.velocity` | H1 · Velocity | float | 0…200 % | 100 % | wird von follow überstimmt |
| `h1.follow` | H1 · Vel. folgt | bool | an/aus | an | an = velocity wird ignoriert |
| `h1.gate` | H1 · Gate | float | 5…100 % | 50 % | relativ zur Step-Länge, an der Step-Kante geklemmt |
| `h1.channel` | H1 · Kanal | int | 1…16 | 2 | nur im Modus Multi |
| `h2.enable` | H2 · Aktiv | bool | an/aus | an |  |
| `h2.degree` | H2 · Grad | int | -14…14 | 4 | diatonische Skalenschritte |
| `h2.octave` | H2 · Oktave | int | -3…3 | 0 | chromatisch, nach degree |
| `h2.velocity` | H2 · Velocity | float | 0…200 % | 100 % | wird von follow überstimmt |
| `h2.follow` | H2 · Vel. folgt | bool | an/aus | an | an = velocity wird ignoriert |
| `h2.gate` | H2 · Gate | float | 5…100 % | 50 % | relativ zur Step-Länge, an der Step-Kante geklemmt |
| `h2.channel` | H2 · Kanal | int | 1…16 | 3 | nur im Modus Multi |
| `h3.enable` | H3 · Aktiv | bool | an/aus | an |  |
| `h3.degree` | H3 · Grad | int | -14…14 | 6 | diatonische Skalenschritte |
| `h3.octave` | H3 · Oktave | int | -3…3 | 0 | chromatisch, nach degree |
| `h3.velocity` | H3 · Velocity | float | 0…200 % | 100 % | wird von follow überstimmt |
| `h3.follow` | H3 · Vel. folgt | bool | an/aus | an | an = velocity wird ignoriert |
| `h3.gate` | H3 · Gate | float | 5…100 % | 50 % | relativ zur Step-Länge, an der Step-Kante geklemmt |
| `h3.channel` | H3 · Kanal | int | 1…16 | 4 | nur im Modus Multi |

## Steps

| ID | Label | Typ | Bereich | Default | Anmerkung |
|---|---|---|---|---|---|
| `s1.active` | S1 · Aktiv | bool | an/aus | an | aus = Slot fällt aus dem Zyklus |
| `s1.mode` | S1 · Modus | choice | 0…3 (Play / Hold / Rest / Ratchet) | Play |  |
| `s1.offset` | S1 · Offset | int | -7…7 | 0 | diatonisch, nur H1–H3 |
| `s1.velocity` | S1 · Velocity | float | 0…200 % | 100 % | Akzente über 100 % |
| `s1.probability` | S1 · Wahrsch. | float | 0…100 % | 100 % | wirkt nur bei Play und Ratchet |
| `s1.ratchet` | S1 · Ratchet | int | 2…8 | 2 | nur im Modus Ratchet |
| `s1.v1` | S1 · Stimme 1 | bool | an/aus | an | Stimmenmaske: Thru |
| `s1.v2` | S1 · Stimme 2 | bool | an/aus | an | Stimmenmaske: H1 |
| `s1.v3` | S1 · Stimme 3 | bool | an/aus | an | Stimmenmaske: H2 |
| `s1.v4` | S1 · Stimme 4 | bool | an/aus | an | Stimmenmaske: H3 |
| `s2.active` | S2 · Aktiv | bool | an/aus | an | aus = Slot fällt aus dem Zyklus |
| `s2.mode` | S2 · Modus | choice | 0…3 (Play / Hold / Rest / Ratchet) | Play |  |
| `s2.offset` | S2 · Offset | int | -7…7 | 0 | diatonisch, nur H1–H3 |
| `s2.velocity` | S2 · Velocity | float | 0…200 % | 100 % | Akzente über 100 % |
| `s2.probability` | S2 · Wahrsch. | float | 0…100 % | 100 % | wirkt nur bei Play und Ratchet |
| `s2.ratchet` | S2 · Ratchet | int | 2…8 | 2 | nur im Modus Ratchet |
| `s2.v1` | S2 · Stimme 1 | bool | an/aus | an | Stimmenmaske: Thru |
| `s2.v2` | S2 · Stimme 2 | bool | an/aus | an | Stimmenmaske: H1 |
| `s2.v3` | S2 · Stimme 3 | bool | an/aus | an | Stimmenmaske: H2 |
| `s2.v4` | S2 · Stimme 4 | bool | an/aus | an | Stimmenmaske: H3 |
| `s3.active` | S3 · Aktiv | bool | an/aus | an | aus = Slot fällt aus dem Zyklus |
| `s3.mode` | S3 · Modus | choice | 0…3 (Play / Hold / Rest / Ratchet) | Play |  |
| `s3.offset` | S3 · Offset | int | -7…7 | 0 | diatonisch, nur H1–H3 |
| `s3.velocity` | S3 · Velocity | float | 0…200 % | 100 % | Akzente über 100 % |
| `s3.probability` | S3 · Wahrsch. | float | 0…100 % | 100 % | wirkt nur bei Play und Ratchet |
| `s3.ratchet` | S3 · Ratchet | int | 2…8 | 2 | nur im Modus Ratchet |
| `s3.v1` | S3 · Stimme 1 | bool | an/aus | an | Stimmenmaske: Thru |
| `s3.v2` | S3 · Stimme 2 | bool | an/aus | an | Stimmenmaske: H1 |
| `s3.v3` | S3 · Stimme 3 | bool | an/aus | an | Stimmenmaske: H2 |
| `s3.v4` | S3 · Stimme 4 | bool | an/aus | an | Stimmenmaske: H3 |
| `s4.active` | S4 · Aktiv | bool | an/aus | an | aus = Slot fällt aus dem Zyklus |
| `s4.mode` | S4 · Modus | choice | 0…3 (Play / Hold / Rest / Ratchet) | Play |  |
| `s4.offset` | S4 · Offset | int | -7…7 | 0 | diatonisch, nur H1–H3 |
| `s4.velocity` | S4 · Velocity | float | 0…200 % | 100 % | Akzente über 100 % |
| `s4.probability` | S4 · Wahrsch. | float | 0…100 % | 100 % | wirkt nur bei Play und Ratchet |
| `s4.ratchet` | S4 · Ratchet | int | 2…8 | 2 | nur im Modus Ratchet |
| `s4.v1` | S4 · Stimme 1 | bool | an/aus | an | Stimmenmaske: Thru |
| `s4.v2` | S4 · Stimme 2 | bool | an/aus | an | Stimmenmaske: H1 |
| `s4.v3` | S4 · Stimme 3 | bool | an/aus | an | Stimmenmaske: H2 |
| `s4.v4` | S4 · Stimme 4 | bool | an/aus | an | Stimmenmaske: H3 |
| `s5.active` | S5 · Aktiv | bool | an/aus | an | aus = Slot fällt aus dem Zyklus |
| `s5.mode` | S5 · Modus | choice | 0…3 (Play / Hold / Rest / Ratchet) | Play |  |
| `s5.offset` | S5 · Offset | int | -7…7 | 0 | diatonisch, nur H1–H3 |
| `s5.velocity` | S5 · Velocity | float | 0…200 % | 100 % | Akzente über 100 % |
| `s5.probability` | S5 · Wahrsch. | float | 0…100 % | 100 % | wirkt nur bei Play und Ratchet |
| `s5.ratchet` | S5 · Ratchet | int | 2…8 | 2 | nur im Modus Ratchet |
| `s5.v1` | S5 · Stimme 1 | bool | an/aus | an | Stimmenmaske: Thru |
| `s5.v2` | S5 · Stimme 2 | bool | an/aus | an | Stimmenmaske: H1 |
| `s5.v3` | S5 · Stimme 3 | bool | an/aus | an | Stimmenmaske: H2 |
| `s5.v4` | S5 · Stimme 4 | bool | an/aus | an | Stimmenmaske: H3 |
| `s6.active` | S6 · Aktiv | bool | an/aus | an | aus = Slot fällt aus dem Zyklus |
| `s6.mode` | S6 · Modus | choice | 0…3 (Play / Hold / Rest / Ratchet) | Play |  |
| `s6.offset` | S6 · Offset | int | -7…7 | 0 | diatonisch, nur H1–H3 |
| `s6.velocity` | S6 · Velocity | float | 0…200 % | 100 % | Akzente über 100 % |
| `s6.probability` | S6 · Wahrsch. | float | 0…100 % | 100 % | wirkt nur bei Play und Ratchet |
| `s6.ratchet` | S6 · Ratchet | int | 2…8 | 2 | nur im Modus Ratchet |
| `s6.v1` | S6 · Stimme 1 | bool | an/aus | an | Stimmenmaske: Thru |
| `s6.v2` | S6 · Stimme 2 | bool | an/aus | an | Stimmenmaske: H1 |
| `s6.v3` | S6 · Stimme 3 | bool | an/aus | an | Stimmenmaske: H2 |
| `s6.v4` | S6 · Stimme 4 | bool | an/aus | an | Stimmenmaske: H3 |
| `s7.active` | S7 · Aktiv | bool | an/aus | an | aus = Slot fällt aus dem Zyklus |
| `s7.mode` | S7 · Modus | choice | 0…3 (Play / Hold / Rest / Ratchet) | Play |  |
| `s7.offset` | S7 · Offset | int | -7…7 | 0 | diatonisch, nur H1–H3 |
| `s7.velocity` | S7 · Velocity | float | 0…200 % | 100 % | Akzente über 100 % |
| `s7.probability` | S7 · Wahrsch. | float | 0…100 % | 100 % | wirkt nur bei Play und Ratchet |
| `s7.ratchet` | S7 · Ratchet | int | 2…8 | 2 | nur im Modus Ratchet |
| `s7.v1` | S7 · Stimme 1 | bool | an/aus | an | Stimmenmaske: Thru |
| `s7.v2` | S7 · Stimme 2 | bool | an/aus | an | Stimmenmaske: H1 |
| `s7.v3` | S7 · Stimme 3 | bool | an/aus | an | Stimmenmaske: H2 |
| `s7.v4` | S7 · Stimme 4 | bool | an/aus | an | Stimmenmaske: H3 |
| `s8.active` | S8 · Aktiv | bool | an/aus | an | aus = Slot fällt aus dem Zyklus |
| `s8.mode` | S8 · Modus | choice | 0…3 (Play / Hold / Rest / Ratchet) | Play |  |
| `s8.offset` | S8 · Offset | int | -7…7 | 0 | diatonisch, nur H1–H3 |
| `s8.velocity` | S8 · Velocity | float | 0…200 % | 100 % | Akzente über 100 % |
| `s8.probability` | S8 · Wahrsch. | float | 0…100 % | 100 % | wirkt nur bei Play und Ratchet |
| `s8.ratchet` | S8 · Ratchet | int | 2…8 | 2 | nur im Modus Ratchet |
| `s8.v1` | S8 · Stimme 1 | bool | an/aus | an | Stimmenmaske: Thru |
| `s8.v2` | S8 · Stimme 2 | bool | an/aus | an | Stimmenmaske: H1 |
| `s8.v3` | S8 · Stimme 3 | bool | an/aus | an | Stimmenmaske: H2 |
| `s8.v4` | S8 · Stimme 4 | bool | an/aus | an | Stimmenmaske: H3 |

## Kanalregeln

- Modus `Multi`: `thru.channel` und `h1…h3.channel` paarweise verschieden,
  die UI verhindert Doppelbelegung.
- Modus `Single`: `thru.channel` ist der Master-Kanal, die `hN.channel`-Werte
  werden ignoriert, bleiben aber gespeichert.
- Umschalten des Modus beendet am nächsten Step-Rand alle Noten auf den alten
  Kanälen (Note-Off) und triggert auf den neuen.
