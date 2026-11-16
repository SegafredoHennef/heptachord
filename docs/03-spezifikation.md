# 03 — Verhaltensspezifikation

Status: **verbindlich für v1** (gilt ab M1 für alle Implementierungen).
Diese Datei ist die *einzige* Verhaltensspezifikation (Entscheidung 50). Der alte
Clone-Plan liegt unverändert im Analyse-Archiv und ist keine Norm.

Quellen der Wahrheit, in dieser Rangfolge:

1. `parameters.toml` — IDs, Bereiche, Defaults (ADR 0003, Entscheidungen 31/52)
2. `docs/entscheidungen.md` — Entscheidungs-Log
3. `docs/adr/0001…0007` — harte, schwer umkehrbare Entscheidungen
4. dieses Dokument — Präzisierung

Archiv-Quelle (nur Funktionswissen, read-only):
`C:\opencode-projekte\astropath\docs\02-behavior.md`. Daraus stammen die
Skalentabellen, der Grad→Halbton-Algorithmus und die 17 Raten. Nichts davon
wurde kopiert — Code, Grafik, Presets und Name des Originals sind neu.

---

## 0. Konventionen

| Begriff | Definition |
|---|---|
| **Sample** | Host-Sample-Frame. 1 Sample = 1 / sampleRate Sekunden. |
| **Pitch** | MIDI-Notennummer 0…127. 60 = mittiges C. |
| **Velocity (intern)** | 1…127. 0 wird nie ausgegeben (MIDI-0 = Note-Off). |
| **Step-Raster** | 8 Slots `s1…s8`, fest. |
| **Aktiver Step** | `sN.active = true`. |
| **Traversal** | Die geordnete Folge der *aktiven* Steps, durch die der Step läuft. |
| **Slot** | Ein Zeitplatz im Raster. Jeder Traversal-Schritt belegt genau einen Slot. |
| **Step-Grenze** | Der Sample, an dem ein Slot beginnt. |
| **Basisgrad** | Der diatonische Grad der Eingangsnote (immer gerastet), bezogen auf den Grundton. |
| **Voice / Stimme** | Thru, H1, H2, H3. Es gibt keine H4. |

Normativ: „muss" = Pflicht, „darf" = frei, „soll" = Empfehlung. Wo dieses
Dokument eine Frage offenlässt, steht ein Marker `[Präzisierung]` mit der
gewählten Lesart — diese Lesart ist verbindlich, bis sie in
`docs/entscheidungen.md` bestätigt oder geändert wird.

---

## 1. Überblick

```
        MIDI-In
            │
            ▼
   ┌──────────────────┐   Note-Tracking: gehaltene Noten,
   │  Eingangsseite   │   letzte Note (Latch), Basisgrad
   └────────┬─────────┘
            │  noteIn, velocityIn
            ▼
   ┌──────────────────┐   Rate, Direction, Swing, Slack,
   │   Step-Engine    │   active/mode/mask/probability
   └────────┬─────────┘
            │  Step-Grenze + aktiver Step
            ▼
   ┌──────────────────┐   root, scale, degree, octave,
   │  Stimmenrechnung  │   offset, velocity, gate
   └────────┬─────────┘
            │  4 × (Note-On, Pitch, Velocity, Gate-Länge, Kanal)
            ▼
        MIDI-Out
```

Der Core kennt weder JUCE noch VST3 (ADR 0007). Er rechnet Sample- und
Event-Zeiten; die Plugin-Schicht übersetzt sie in MIDI-Events mit
Sample-Offset.

**Kernregel:** Audio wird nicht verarbeitet. Das Plugin ist ein Instrument mit
stillem Stereo-Bus (ADR 0001); der Standalone ergänzt einen Test-Synth.

---

## 2. Eingangsseite

### 2.1 Note-Tracking

- Das Plugin ist **monophon mit Last-Note-Priority** (Entscheidung 5).
  Bei mehreren gehaltenen Noten zählt die zuletzt gedrückte. Wird sie
  losgelassen, fällt die Tonhöhenquelle auf die davor gehaltene Note zurück —
  samt deren Velocity; Pitch und Velocity gehören immer zur selben Note.
- Ein Note-On startet den Betrieb des Step-Engines *nicht* neu: die Uhr läuft
  ab Transport-Start kontinuierlich weiter (Entscheidung 13).
- Solange keine Note gehalten wird (und Latch nichts hält), entstehen **keine
  Gates** — die Schritte laufen leer durch.

### 2.2 Latch

- `latch = true`: die zuletzt gespielte Note bleibt als Tonhöhenquelle erhalten,
  auch nach dem Loslassen der Taste. Die gehaltene Note zählt weiterhin als
  „gehalten", der Step-Engine liefert also weiter Gates.
- Latch wirkt **nicht** auf Gate-Längen: ein Gate endet immer nach
  `hN.gate` bzw. an der Step-Grenze (Entscheidung 15, 33).
- Latch wird gelöscht durch: `latch` auf `false`, `reset`, Preset-Wechsel.

### 2.3 Rohnote vs. Basisgrad

- **Rohnote** = die gehaltene (ggf. gelatchte) Eingangsnote, unverändert.
- **Basisgrad** = der diatonische Grad der Rohnote, bezogen auf den Grundton.
  Die Rohnote wird dazu **immer** auf den nächsten Skalenton gerastet
  (Entscheidung 5). Bei Gleichstand gewinnt der tiefere Ton (siehe 3.4).
- `thru.snap` wirkt nur auf die **Thru-Stimme**: `false` (Default) = Thru
  spielt die Rohnote, `true` = Thru spielt den gerasteten Ton.
  Auf die Harmonien hat `thru.snap` keinen Einfluss — die sind immer Skalen-
  töne.

---

## 3. Skalen und Tonhöhen

### 3.1 Skalen

Acht Skalen, Reihenfolge wie im Original (identifiziert aus den Skalentabellen
im Binary, `02-behavior.md` §2). Der Index ist ein diskreter Parameter.

| Index | Skala | Halbton-Offsets je Oktave |
|---|---|---|
| 0 | Dur (Ionisch) | 0 2 4 5 7 9 11 |
| 1 | Moll (Äolisch) | 0 2 3 5 7 8 10 |
| 2 | Harmonisch Moll | 0 2 3 5 7 9 11 |
| 3 | Melodisch Moll | 0 2 3 5 7 9 11 |
| 4 | Dorisch | 0 2 3 5 7 9 10 |
| 5 | Phrygisch | 0 1 3 5 7 8 10 |
| 6 | Lydisch | 0 2 4 6 7 9 11 |
| 7 | Mixolydisch | 0 2 4 5 7 9 10 |

Phrygisch enthält mit der *b2* (Halbton 1) den characteristic „Phrygian" — das
ist kein Tippfehler, siehe `src/core/Scales.cpp`.

Lokrisch gibt es nicht — im Original bewusst weggelassen, in Heptachord
ebenfalls nicht.

### 3.2 Grad → Halbton

Der musikalische Kern (aus `02-behavior.md` §2 rekonstruiert, in
`src/core/Scales.cpp` implementiert):

```
oktav = floorDiv(grad, 7)
index = grad − 7 · oktav            (0…6)
semitone = 12 · oktav + skala[index]
```

`floorDiv` rundet **immer nach unten** (C++- `/` gegen Null). Negative Grade
sind damit symmetrisch: Grad −1 = eine Stufe unter dem Grundton.

Der Halbtonwert ist relativ zum Grundton. Die absolute Note ist:

```
note = rootPitch + semitone
```

mit `rootPitch` = MIDI-Nummer des Grundtons (`root`: C = 0 … B = 11).

### 3.3 Basisgrad der Eingangsnote

```
rel       = noteIn − rootPitch
oktav     = floorDiv(rel, 12)
inner     = rel − 12 · oktav
index     = größtes i mit skala[i] ≤ inner      (gerastet, Gleichstand → tieferer Ton)
baseGrad  = 7 · oktav + index
```

Der Grad zählt vom **tiefsten Vorkommen des Grundtons** (MIDI `root … root+11`).
Grad 0 ist damit der Grundton selbst, Grad 7 eine Oktave darüber.

Beispiel: Grundton C, Skala Dur, Eingangsnote 63 (Es) → gerastet auf 62 (D) →
`rel = 62 − 0 = 62` → `oktav = floorDiv(62, 12) = 5` → `inner = 2` →
`index = 1` → `baseGrad = 7·5 + 1 = 36`.

Der absolute Wert des Basisgrads ist beliebig, solange er konsistent ist: Die
Pitch-Rechnung in 3.5 addiert dazu `hN.degree` und `sN.offset`, sodass sich
Octaven und Bezugston am Ende aufheben.

### 3.4 Rasten (Snap)

```
snapToScale(note, scale, root):
    Kandidaten = alle Töne root + 12·k + skala[i]   (k … , i = 0…6)
    Abstand    = |Kandidat − note|
    gewinnt    = kleinster Abstand; bei Gleichstand der tiefere Ton
```

Regel: **bei Gleichstand gewinnt der tiefere Ton** (dokumentierte Festlegung;
der Test `snapToScale(63, Dur) == 62` in `tests/test_scales.cpp` prüft das).

> **Abweichung zur aktuellen Implementierung:** `snapToScale` in
> `src/core/Scales.cpp` rastet bisher **wurzelunabhängig** (implizit Root C).
> Die Spezifikation verlangt eine **root-relative** Rastung. M1 ändert die
> Signatur zu `snapToScale(note, scale, root)` und passt die Tests an
> (siehe Abschnitt 12).

### 3.5 Pitch der Stimmen

```
grad_i   = baseGrad + h_i.degree + sN.offset        (diatonisch, sN.offset nur H1–H3)
semitone = degreeToSemitone(grad_i, scale)
pitch_i  = clamp(rootPitch + semitone + 12 · h_i.octave, 0, 127)
```

- `h_i.degree`: −14…+14 diatonische Stufen (Entscheidung 34).
- `sN.offset`: −7…+7 diatonische Stufen, wirkt **nur** auf H1–H3.
- `h_i.octave`: −3…+3 chromatische Oktaven, **nach** dem Grad.
- Die Thru-Stimme hat keinen Grad und keine Oktave; ihre Pitch ist die Rohnote
  (gefiltert nach `thru.snap`).

### 3.6 Gültigkeitsbereich

`pitch_i` wird hart auf 0…127 geklemmt. Grade außerhalb des MIDI-Bereichs
erzeugen also keine Fehler, sondern klemmende Töne am Rand.

---

## 4. Raten und Step-Timing

### 4.1 Die 17 Raten

Musikalische Teiler (kein Hz-Wert), Reihenfolge wie im Original
(`02-behavior.md` §3):

| Wert | Bedeutung | Wert | Bedeutung |
|---|---|---|---|
| 0.0625 | 1/16 | 0.375 | 3/8 |
| 0.0833 | 1/12 (Triole) | 0.5 | 1/2 |
| 0.125 | 1/8 | 0.6667 | 2/3 (Triole) |
| 0.1667 | 1/6 (Triole) | 0.75 | 3/4 |
| 0.25 | 1/4 | 1.0 | 1 Takt |
| 0.3333 | 1/3 (Triole) | 1.3333 | 4/3 (Triole) |
| | | 1.5 | 3/2 |
| | | 2.0 | 2 |
| | | 3.0 | 3 |
| | | 4.0 | 4 |
| | | 8.0 | 8 |

### 4.2 Schrittlänge

```
stepSeconds = rate × 4 × 60 / BPM
```

`rate = 1.0` bedeutet **ein Takt (4/4)** (Entscheidung 21). Der Faktor 4
kommt aus der Vierer-Takt-Definition; er ist eine Entscheidung von uns, nicht
aus dem Binary belegbar, und steht deshalb hier.

Tempoquelle: der Host (VST3-Transport). Fehlt eine Tempoangabe, gilt 120 BPM.

### 4.3 Slot-Zeiten

Die Slot-Nummer `n` zählt **absolut** ab Transport-Start, nicht pro Zyklus:

```
T(n) = n · stepSeconds + (n ungerade ? swingSeconds : 0) + slackSeconds
```

- `swingSeconds = swing / 100 · stepSeconds` (Swing 0…75 %).
  Ungerade Slots (2., 4., …) sind **verzögert** (Entscheidung 14).
  Bei 75 % bleiben die Slots monoton: Abstand ungerade→gerade = 25 % der
  Schrittlänge.
- `slackSeconds = slack` in Millisekunden, −100…+100 ms, **global** und
  additiv für alle Slots (Entscheidung 14).
- Slack wird auf weniger als eine Schrittlänge geklemmt:
  `|slackSeconds| < stepSeconds`. Bei sehr hoher Rate greift die Klemme.

Slot-Zeiten sind monoton steigend; jeder Traversal-Schritt nimmt den nächsten
Slot.

### 4.4 Reset

`reset` ist ein Trigger (Entscheidung 39). Wirkung: der **nächste** Step-Rand
beginnt wieder bei Slot 0 / dem ersten aktiven Step der Traversal-Folge. Ein
laufender Slot-Timer wird dabei nicht zurückgesetzt — nur die Zuordnung
Slot → Step.

### 4.5 Locate und Loop-Sprung

Die Slot-Position wird aus der Host-ppq abgeleitet (Entscheidung 64): Slot `n`
beginnt bei `T(n)` (Abschnitt 4.3), die Position im Block ist
`(T(n) − ppq) · sampleRate · BPM/60`. Weicht die Host-ppq um mehr als eine
Viertel-Slotlänge vom fortgeschriebenen Wert ab (Locate, Loop-Sprung,
Tempoänderung), gilt der Host:

- klingende Noten werden per **All-Notes-Off** beendet (Abschnitt 8.3),
- die Traversal-Folge beginnt neu beim ersten aktiven Step,
- die Slot-Zuordnung folgt wieder `T(n)`.

Innerhalb eines Blocks werden alle Ereignisse sample-genau einsortiert; der
Rasterbezug bleibt dadurch auch über Blockgrenzen hinweg stabil.

### 4.6 Genauigkeit

Step-Grenzen liegen auf Sample-Grenzen; Jitter ≤ 1 Sample. Der Plugin meldet
**0 Samples Latenz** und verarbeitet keinen Audioinhalt.

---

## 5. Step-Engine

### 5.1 Zyklus und Richtung

Der Zyklus besteht aus den **aktiven** Steps (`sN.active = true`), in
Rasterreihenfolge. Inaktive Steps fallen aus dem Zyklus (Entscheidung 36) — das
betrifft auch PingPong und Zufall, die ausschließlich über aktive Steps laufen.

| `direction` | Folge der Traversal-Positionen |
|---|---|
| Vorwärts | `0, 1, …, k−1` (zyklisch) |
| Rückwärts | `k−1, k−2, …, 0` (zyklisch) |
| PingPong | `0, 1, …, k−1, k−2, …, 1, 0, 1, …` — Endpunkte jeweils einmal |
| Zufall | pro Schritt eine zufällige Position in `0…k−1` (mit Wiederholung) |

`k` = Anzahl aktiver Steps. Ändern sich die `active`-Flags, wird die Liste neu
gebildet; der Cursor wird auf die gültige Position geklemmt.

> `[Präzisierung]` Zufall: **mit** Wiederholung — dieselbe Step-Zelle kann
> zweimal hintereinander kommen. Ohne Wiederholung wäre die Folge ein
> Zufalls-Hamilton-Pfad, der beim Zurücksetzen hängen kann.

### 5.2 Step-Modi

Jeder Step hat genau einen Modus. Alle vier Stimmen folgen demselben Modus
(ADR 0004).

| Modus | Wirkung |
|---|---|
| **Play** | Ein Trigger am Step-Rand. Die im Maske aktivierten Stimmen werden neu angeschlagen (Gate-Länge = `hN.gate`). |
| **Hold** | Der Trigger wird **einmal** gesetzt; die Note wird über die Step-Grenze hinweg gehalten. Folge-Hold-Steps schlagen nicht erneut an. Beendet wird die Kette, sobald ein Step kommt, in dem die Stimme nicht maskiert ist oder in dem Modus Play/Ratchet gilt. |
| **Rest** | Kein Trigger, kein Ton. Der Slot bleibt im Raster stehen (im Unterschied zu `active = false`, das den Slot ganz entfernt). |
| **Ratchet** | `sN.ratchet` (2…8) Trigger innerhalb des Slots, gleichmäßig verteilt: Treffer `k` (0-basiert) bei `Step-Grenze + k · stepSeconds / ratchet`. |

Ratchet-Details:

- Jeder Treffer ist ein eigener Note-Off/Note-On-Paar (kein Überlappen auf
  demselben Kanal).
- Die Gate-Länge eines Ratchet-Treffers wird zusätzlich auf
  `stepSeconds / ratchet` geklemmt (kein Überlappen, Entscheidung 33).
- Hold und Rest haben kein Ratchet; `sN.ratchet` wird dort ignoriert.

### 5.3 Stimmenmaske

Pro Step vier Einzel-Bools `sN.v1…v4` (Thru, H1, H2, H3) (Entscheidung 23/37).
Die Maske entscheidet, **welche Stimmen** in diesem Step anschlagen. Sie
kombiniert sich mit `thru.enable` und `h_i.enable`:

```
voiceActive(i, step) = enable_i AND sN.v(i)
```

Die Maske ist im Parameterraum 32 Einzel-Bools; intern eine Bitmaske.

### 5.4 Wahrscheinlichkeit

`sN.probability` (0…100 %) wirkt **nur** bei Play und Ratchet (Entscheidung 25):

- Pro Trigger-Ereignis ein Wurf `r ∈ [0, 1)`. Der Trigger findet statt, wenn
  `r < probability / 100`. Bei 100 % findet er immer statt.
- Der Wurf ist **pro Stimme und pro Treffer** (Entscheidung 65): Bei einem
  Ratchet-Treffer können Thru und H1 anschlagen, H2 und H3 ausfallen.
- Bei Ratchet ist der Wurf **pro Treffer** — einzelne Treffer dürfen ausfallen.
- Hold und Rest würfeln nicht.

Ein ausgefallener Treffer ändert **nicht** die Gate-Länge der übrigen Stimmen;
die Gate-Zeit hängt nur von `hN.gate`, `stepSeconds` und `ratchet` ab.

---

## 6. Stimmen, Velocity und Gate

### 6.1 Stimmen

| Stimme | Pitch | Parameter |
|---|---|---|
| Thru | Rohnote (optional gerastet) | `thru.*` |
| H1 | `baseGrad + h1.degree + sN.offset` | `h1.*` |
| H2 | `baseGrad + h2.degree + sN.offset` | `h2.*` |
| H3 | `baseGrad + h3.degree + sN.offset` | `h3.*` |

Es gibt keine H4 (Original-Konstrukt: Thru + 3 Harmonien).

### 6.2 Velocity

Alle Velocity-Werte sind **Prozent** (Entscheidung 32). Intern wird
multipliziert und auf 1…127 geklemmt.

```
v_in      = Note-On-Velocity (1…127)
f_step    = sN.velocity / 100                      (0…2)
f_thru    = thru.velocity / 100                    (0…2)
f_harm    = harm.velocity / 100                    (0…2)
f_voice(i)= h_i.follow ? 1 : h_i.velocity / 100    (0…2)
f_spread(i)= 1 − spread · (i−1)/2                  (i = 1…3)

v_thru = clamp(round(v_in · f_thru · f_step), 1, 127)
v_h(i) = clamp(round(v_in · f_harm · f_voice(i) · f_spread(i) · f_step), 1, 127)
```

- `thru.velocity` und `harm.velocity` sind die **Gruppenfaktoren**: der erste
  für die Thru-Stimme, der zweite für H1–H3. Sie bilden die beiden
  Velocity-CV-Eingänge des Originals ab (`CV_In_ThruVel`, `CV_In_HarmVel`).
- `h_i.velocity` ist der **Stimmentrim** und wirkt nur, wenn
  `h_i.follow = false` ist.

- `harm.spread` wirkt **nur** auf H1–H3. Default 0.3 → 100 % / 85 % / 70 %
  (Entscheidung 35). Bei `spread = 1` wird H3 zu 0 % (stumm) — das ist gewollt
  und wird in der UI als „H3 stumm" angezeigt.
- `sN.velocity` ist ein Akzentfaktor des Steps und wirkt auf **alle** Stimmen
  des Steps, auch auf Thru. Werte über 100 % sind Betonungen.
- `h_i.follow = true` (Default): die Voice-Parameter `h_i.velocity` werden
  **ignoriert**, die Stimme spielt die Eingangs-Velocity (mal Spread, mal
  Step-Akzent). `follow = false`: `h_i.velocity` wirkt als Faktor auf die
  Eingangs-Velocity.

> `[Präzisierung]` Die frühere Formulierung „`hN.velocity = 0` folgt der
> Eingabe" (Entscheidung 9) ist durch den Schalter `follow` ersetzt. Der
> Wert 0 % ist mit `follow = false` ein Sonderfall: das Ergebnis wird auf 1
> geklemmt (also praktisch stumm, aber nicht ganz stumm).

### 6.3 Gate

```
gateLen(i) = h_i.gate / 100 · stepSeconds        (5 % … 100 %)
```

- Der Wert ist die **relative Gate-Länge** innerhalb des Steps.
- Das Gate endet **spätestens an der Step-Grenze** — es überlappt nie in den
  nächsten Step hinein (Entscheidung 33). Bei 100 % endet es exakt an der
  Grenze.
- Die Thru-Stimme hat in v1 **keinen** Gate-Parameter; ihr Gate ist fest 100 %
  der Schrittlänge. Ein Parameter `thru.gate` ist ein Kandidat für v1.1 und
  würde eine `schemaVersion`-Erhöhung brauchen (Entscheidung 40).
- In Hold läuft das Gate nicht ab (siehe 5.2). In Ratchet wird die Gate-Länge
  zusätzlich auf `stepSeconds / ratchet` geklemmt.

---

## 7. Kanäle und Ausgabe

`output.mode` (ADR 0005):

| Modus | Kanalzuordnung |
|---|---|
| **Multi** (Default) | Thru → `thru.channel`, H1 → `h1.channel`, H2 → `h2.channel`, H3 → `h3.channel` |
| **Single** | Alle vier Stimmen → `thru.channel` (Master). `hN.channel` wird ignoriert, die Werte bleiben gespeichert (Entscheidung 38). |

- Im Modus Multi müssen die vier Kanäle **paarweise verschieden** sein. Die
  UI meldet eine Doppelbelegung als Fehler; das Verhalten ändert sich nicht
  (kein automatisches Umhängen — der Host soll entscheiden).
- Die Kanäle sind 1…16.

---

## 8. Noten-Lebenszyklus, Retune, All-Notes-Off

### 8.1 Ereignisreihenfolge an einer Step-Grenze

Pro Stimme in der Reihenfolge Thru, H1, H2, H3:

1. **Note-Off** der bisher klingenden Note auf diesem Kanal (falls vorhanden),
2. **Note-On** mit Pitch, Velocity und Gate-Länge.

Es gibt **kein Pitchbend**: ein Pitchwechsel ist immer Note-Off → Note-On
(Entscheidung 18).

Ein Gate-Ende erzeugt genau einen Note-Off auf dem Kanal der Stimme.

### 8.2 Retune-Regel

Änderungen an `root`, `scale`, `hN.degree`, `hN.octave` wirken **am nächsten
Step-Rand** (Entscheidung 17). Eine Änderung mitten im Step verändert keine
klingende Note; sie zeigt sich erst im nächsten Trigger.

Ausnahmen, die sofort wirken und mit Note-Off/All-Notes-Off behandelt werden:
Preset-Wechsel, `reset` (nur Position), `output.mode`-Wechsel und jeder
Kanalwechsel (`thru.channel`, `hN.channel`). Beim Kanalwechsel bekommen die
**verlassenen** Kanäle sofort ihr All-Notes-Off, damit keine Note hängen bleibt.

### 8.3 All-Notes-Off

Ein All-Notes-Off (je belegtem Kanal) geht raus bei:

- Transport-Stop,
- Locate / Loop-Sprung (Abschnitt 4.5),
- Preset-Wechsel (Laden),
- Wechsel der Kanalbelegung (Abschnitt 8.2),
- Plugin-Deaktivierung / Editor-Schließen.

Im Modus Multi sind das die Kanäle von Thru und H1–H3, im Modus Single nur
`thru.channel` (Entscheidung 38).

### 8.4 Ereignisschicht

Der Core erzeugt pro Block eine Liste `TimedEvent` (Offset, Typ, Pitch,
Velocity, Kanal). Sie ist fest dimensioniert (1024 Einträge) und allokiert
nicht im Audiothread. Ein Block wird in **einem** Durchlauf abgearbeitet; für
jedes Sample, an dem etwas passiert, gilt diese Reihenfolge:

1. **fällige Gate-Enden** (Note-Off),
2. **Eingabe-Events** des Hosts (Note-On/Note-Off, Offset im Block),
3. **Ratchet-Treffer** des laufenden Slots,
4. **Slot-Beginn** (neuer Step).

Damit steht an derselben Sample-Position jedes Note-Off vor dem zugehörigen
Note-On (Abschnitt 8.1), und ein eintreffendes Note-Off wirkt noch vor dem
Trigger an dieser Grenze.

Gate-Enden und laufende Slots werden über die Blockgrenze hinaus
weitergereicht: Ein Gate, das hinter dem Blockende liegt, wird nicht
abgeschnitten, sondern im Folgeblock fällig. Ratchet-Treffer jenseits des
Blockendes ebenso.

Eingabe-Events werden auch verarbeitet, wenn der Transport steht (Notentracking
und Latch bleiben aktuell); Trigger entstehen dann keine.

Der `reset`-Trigger wirkt auf den **nächsten** Slot-Beginn (Abschnitt 4.4), ein
Preset-Wechsel oder `requestAllNotesOff()` sofort an Offset 0 des nächsten
Blocks.

---

## 9. Presets und Zustand

- Presets sind JSON-Dateien mit Endung `.heptachord`
  (ADR 0006), `schemaVersion` ganzzahlig ab 1 (Entscheidung 46).
- Eine höhere `schemaVersion` ist ein **Fehler**: das Preset wird abgelehnt,
  nicht teilweise geladen.
- JSON wird ausschließlich in der Plugin-Schicht gelesen/geschrieben
  (`juce::var`); der Core kennt ein flaches Struct.
- Der VST3-Zustand ist dasselbe JSON.
- Parameter-IDs sind eingefroren (`tests/parameter_ids.txt`, Entscheidung 40).
  Eine Änderung braucht eine `schemaVersion`-Erhöhung **und** eine
  Migrationsregel im Preset-Loader.
- Acht Factory-Presets, je eines pro Skala (Entscheidung 19).

---

## 10. Standalone

- Eigener Transport: 120 BPM, Leertaste oder Knopf startet/stoppt
  (Entscheidung 29). Im Standalone läuft die Uhr als einzige Quelle; es gibt
  keinen Host, dessen ppq überschreiben könnte.
- Eingebauter Test-Synth: 4-stimmig, Sägezahn, Tiefpass, ADSR (Entscheidung 69).
  Er spielt genau die vier Stimmen, die der Core auf vier Kanälen ausgibt.
- MIDI-Clock-Eingang erst ab M6.
- Der Standalone nutzt denselben Core und denselben Editor. Nur der
  Standalone-Wrapper schreibt Audio; der VST3-Ausgang bleibt still.

---

## 11. Nicht in v1

Mod-Matrix, Free-Rate (Hz), MPE, MIDI-Learn, Per-Step-Slack, H4+, Tastatur-Split,
Reason-CV-Pfade, CLAP, macOS, Signierung.

Die sechs CV-Funktionen des Originals werden in v1 als **host-automatisierbare
Parameter** bzw. als feste Funktion abgebildet:

| Original-CV | Heptachord v1 |
|---|---|
| `CV_In_Slack` | `slack` (Parameter) |
| `CV_In_ThruVel` | `thru.velocity` (Parameter) |
| `CV_In_HarmVel` | `harm.velocity` (Parameter) |
| `CV_In_ActiveSteps` | `sN.active` (Parameter) |
| `CV_In_StepTrig` | entfällt — `reset` bleibt (Entscheidung 39) |
| `CV_In_Reset` | `reset` (Trigger) |

Die acht CV-Ausgänge (Pitch/Gate pro Stimme) werden durch MIDI-Out auf
4 Kanälen abgebildet.

---

## 12. Abweichungen zur aktuellen Implementierung

| # | Abweichung | Zielzustand | Stand |
|---|---|---|---|
| A1 | `snapToScale` rastet wurzelunabhängig (Root C) | root-relative Rastung | **erledigt in M1**: `snapToScale(note, scale, root)` |
| A2 | `h_i.follow` ist noch nicht implementiert | Velocity-Formel aus 6.2 | **erledigt in M1** |
| A3 | Step-Engine fehlt vollständig | Abschnitt 4/5 | **erledigt in M1**: `StepClock`, `StepSequencer`, `StepPlanner` |
| A4 | Thru-Gate ist nicht konfigurierbar | 100 % fix | offen (bewusst so, siehe 6.3) |
| A5 | Kein Velocity-Faktor pro Stimme im Core | Abschnitt 6 | **erledigt in M1** |
| A6 | Keine Note-Eingangsseite | Abschnitt 2.1/2.2 | **erledigt in M2a**: `NoteTracker` |
| A7 | Keine Ereignisschicht (nur blockweise Planung) | Abschnitte 4.5, 8.4 | **erledigt in M2a**: `StepEngine`, `TimedEvent` |
| A8 | Parameter, Transport und MIDI-Ausgabe sind noch nicht angebunden | — | **erledigt in M2b**: `ParameterBinding`, `ProcessorBridge` |
| A9 | Kein hörbarer Ton im Standalone | Abschnitt 10 | **erledigt in M2b**: `TestSynth` |

Der Core enthält `StepClock` (Timing), `StepSequencer` (Traversal),
`StepPlanner` (Planung + Voice-Mathematik), `NoteTracker` (Eingangsseite),
`StepEngine` (Ereignisschicht) und `SettingsFromValues` (Parameterabbildung).
Die Plugin-Schicht ist dünn: `ParameterBinding` (APVTS → Werte),
`ProcessorBridge` (Werte + Host-Zeit → MIDI) und `TestSynth` (nur Standalone).
Damit ist der Signalweg vollständig; offen ist der Editor (M3) und der
Preset-Loader (JSON, M5).

---

## 13. Abnahmekriterien

Jedes Kriterium ist maschinell prüfbar; die Nummerierung ist die
Referenz für die Testdateien.

| # | Kriterium | Prüfung |
|---|---|---|
| T1 | 12 Eingangstöne × 8 Skalen × 4 Stimmen ergeben die korrekten diatonischen Intervalle | Python-Referenz (Abschnitt 14) als CSV, vom C++-Test gelesen |
| T2 | Grad→Halbton: negativ, 0, ±7, ±14 stimmen | Unit-Test gegen Tabelle |
| T3 | Snap bei Gleichstand liefert den tieferen Ton | Unit-Test |
| T4 | Root-relative Rastung: Root Cis rastet auf Cis-Dur | Unit-Test (M1, A1) |
| T5 | `stepSeconds` entspricht `rate × 4 × 60 / BPM` | Unit-Test über alle 17 Raten |
| T6 | Swing verschiebt ungerade Slots um `swing %` der Schrittlänge | Unit-Test |
| T7 | Slack wird additiv angewandt und auf < 1 Schrittlänge geklemmt | Unit-Test |
| T8 | Ratchet erzeugt `sN.ratchet` Trigger im Slot | Unit-Test |
| T9 | Rest erzeugt keinen Trigger, `active = false` verlässt den Zyklus | Unit-Test |
| T10 | PingPong läuft nur über aktive Steps | Unit-Test |
| T11 | Probability 0 % erzeugt nie einen Trigger, 100 % immer | Unit-Test (seeded) |
| T12 | Velocity: `round(v · Faktor)`, geklemmt auf 1…127; Spread-Folge 100/85/70 | Unit-Test |
| T13 | Gate endet nie nach der Step-Grenze | Unit-Test |
| T14 | Kanäle: Multi = 4 Kanäle, Single = alle auf `thru.channel` | Unit-Test |
| T15 | Pitchwechsel = Note-Off + Note-On, kein Pitchbend | Event-Test (`test_engine.cpp`, M2a) |
| T16 | Preset-Roundtrip: Zustand → Datei → Zustand ist identisch | Integrationstest |
| T17 | Step-Kanten liegen auf Sample-Grenzen, Jitter ≤ 1 Sample | Event-Test (`test_engine.cpp`, M2a) |
| T18 | Parameter-IDs unverändert | Golden File `tests/parameter_ids.txt` + CI |

---

## 14. Python-Referenz

Unabhängige Implementierung in Python 3.12 (nur Standardbibliothek), die die
Tabelle 12 × 8 × 4 als CSV erzeugt. Die CSV wird **eingecheckt**; CI
regeneriert sie und schlägt bei Abweichung fehl (Entscheidung 47).

Spalten: `inputNote, scale, voice, expectedDegree, expectedSemitone`.
Zeilen: 12 Eingangstöne × 8 Skalen × 4 Stimmen = 384 Datenzeilen plus Kopf.

---

## Anhang A — Formeln auf einen Blick

```
stepSeconds = rate · 4 · 60 / BPM
T(n)        = n · stepSeconds + (n ungerade ? swing/100 · stepSeconds : 0) + slack

baseGrad    = 7 · floorDiv(noteIn − rootPitch, 12) + rastIndex(noteIn)

grad_i      = baseGrad + h_i.degree + sN.offset
pitch_i     = clamp(rootPitch + degreeToSemitone(grad_i, scale) + 12 · h_i.octave, 0, 127)

v_thru      = clamp(round(v_in · thru.velocity/100 · sN.velocity/100), 1, 127)
v_h(i)      = clamp(round(v_in · (h_i.follow ? 1 : h_i.velocity/100)
                             · (1 − spread·(i−1)/2) · sN.velocity/100), 1, 127)

gateLen(i)  = h_i.gate/100 · stepSeconds          (≤ stepSeconds)
```

## Anhang B — Herkunft der Zahlen

| Angabe | Herkunft |
|---|---|
| 8 Skalen + Tabellen | Binary-Analyse, `astropath/docs/02-behavior.md` §2 |
| Grad→Halbton, floorDiv | Binary-Analyse, dort §2; Implementierung `src/core/Scales.cpp` |
| 17 Raten | Binary-Analyse, dort §3 |
| 4 Stimmen (Thru + H1–H3) | Binary-Analyse, dort §4 |
| 8 CV-Ein-/Ausgänge | Binary-Analyse, dort §5; Zuordnung v1 → Abschnitt 11 |
| 8 Steps | Binary-Analyse (8 Step-Anzeigen/LEDs) |
| `rate = 1.0` = ein Takt | unsere Festlegung, Entscheidung 21 |
| Alle Parameter, Bereiche, Defaults | `parameters.toml` |
| Kanalmodell, Step-Gating, Identity | ADR 0001–0007 |
| Gate 5–100 %, Velocity in Prozent | Entscheidungen 32, 33 |
| `follow`-Semantik, root-relative Rastung | Präzisierung dieses Dokuments, Abschnitt 12 |
