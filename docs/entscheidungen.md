# Entscheidungs-Log

Kurzprotokoll der Grill-Runden. ADRs nur für harte, schwer umkehrbare
Entscheidungen; alles andere steht hier.

## Runde 1

1. **Produktform** — ein Build, kein Audio im Plugin; Standalone mit Test-Synth.
   Siehe ADR 0001 (in Runde 2 auf Instrument mit stillem Bus korrigiert).
2. **Lizenz** — AGPLv3, öffentliches Repo. Siehe ADR 0002.
3. **Plattform/Hosts v1** — Windows 10/11, VST3; Live, Reaper, Bitwig, Cubase.
   macOS/CLAP nach v1.
4. **Projektstart** — neues Repo `Heptachord`, `astropath` bleibt Analyse-Archiv;
   JUCE als Submodule, Catch2 per FetchContent.
5. **Eingangsnote** — monophon, Last-Note-Priority; Basisgrad gerastet; Thru
   gibt die Rohnote aus.
6. **Step-Modi** — Play, Hold, Rest, Ratchet; `sN.offset` transponiert nur
   H1–H3.
7. **v1-Schnitt** — ohne Mod-Matrix, Free-Rate, MPE, MIDI-Learn; die sechs
   CV-Funktionen werden host-automatisierbare Parameter.
8. **Parameter/Preset** — deklarative Tabelle als SoT, JSON-Presets mit
   `schemaVersion`. Siehe ADR 0003.
9. **Velocity/Gate** — multiplikativ, `hN.velocity = 0` folgt der Eingabe.

## Runde 2

10. **Thru-Gate** — alle vier Stimmen sind gestept. Siehe ADR 0004.
11. **Deklaration** — VST3-Instrument mit stillem Stereo-Bus, Kategorie
    `Instrument|Synth`. Siehe ADR 0001.
12. **Kanäle** — `output.mode` `Multi` (1–4) / `Single`. Siehe ADR 0005.
13. **Clock** — läuft ab Transport-Start kontinuierlich; Gates nur bei
    gehaltener Note; `reset` setzt am nächsten Step-Rand auf Step 1.
14. **Swing/Slack** — Swing verzögert ungerade Steps bis 75 % der Step-Länge;
    Slack global −100…+100 ms, additiv, auf < 1 Step geklemmt; kein
    Per-Step-Slack in v1.
15. **Latch** — hält die zuletzt gespielte Note, hält nur die Eingangsnote,
    nicht die Gates; Aus/`reset`/Preset-Wechsel löscht sie.
16. **Split** — `keyboardSplit` gestrichen; `thru.snap` = „Thru mitrasten",
    Default aus.
17. **Retune** — Skalen-/Grundton-/Preset-Wechsel wirken am nächsten Step-Rand.
18. **Notenhaushalt** — Kanäle paarweise verschieden; Pitchwechsel per
    Note-Off/Note-On am Step-Rand, kein Pitchbend; All-Notes-Off bei Stop und
    Preset-Load.
19. **Undo/Presets** — nur Host-Undo; 8 Factory-Presets (eine Referenz je
    Skala), 4 Kombinationen in M5.
20. **Test-Referenz** — unabhängige Python-Implementierung erzeugt die
    12×8×4-CSV; C++-Tests lesen sie; dazu Property-Tests.

## Runde 3

21. **Rate-Basis** — 1.0 = ein Takt (4/4), also
    `stepSeconds = rate × 4 × 60/BPM`; unsere Definition, im Binary nicht
    beweisbar.
22. **Name** — *Heptachord*, Hersteller „Blakkmoon Audio", Bundle-ID
    `com.blakkmoon.heptachord`. Siehe ADR 0006.
23. **Stimmenmaske** — pro Step und Stimme, vier Bools (`sN.v1…v4`).
24. **Step-Modus** — pro Step global; `sN.ratchet` 2…8 nur bei Modus Ratchet.
25. **Probability** — in v1, Default 100 %, wirkt nur bei Play/Ratchet.
26. **degree/octave** — `degree` diatonisch, `octave` chromatisch danach.
27. **GUI** — zwei Ebenen; Step-Detail als ausklappbares Panel unter dem Grid.
28. **Default-Preset** — C Dur, 1/8, H1 +2 / H2 +4 / H3 +6 (Cmaj7), Gate 50 %.
29. **Standalone** — eigener Transport (120 BPM, Leertaste), MIDI-Clock erst
    M6; Test-Synth 4-stimmig (Sägezahn, Tiefpass, ADSR).

## Runde 4

30. **Name/Identität** — Heptachord, `com.blakkmoon.heptachord`, „Blakkmoon
    Audio", Preset-Endung `.heptachord`. Siehe ADR 0006.
31. **Parameter-Quelle** — `parameters.toml` ist die SoT; `docs/02-parameter.md`
    wird daraus generiert. Siehe ADR 0003.
32. **Velocity-Einheiten** — überall Prozent, 0…200 %, Default 100 %; intern
    multiplikativ, geklemmt auf 1…127.
33. **Gate** — 5…100 %, hart an der Step-Kante geklemmt; kein Überlappen in v1.
34. **`sN.offset`** — diatonisch, ±7 Skalenschritte; einzige Chromatizität
    bleibt `hN.octave`.
35. **`harm.spread`** — `spread_i = 1 − spread × (i−1)/2`, i = H1…H3
    (Default 0.3 → 100/85/70 %).
36. **`sN.active` vs. Rest** — beides bleibt: inaktiv nimmt den Slot aus dem
    Zyklus (PingPong/Random laufen nur über aktive Steps), Rest lässt das Raster
    stehen.
37. **Stimmenmaske** — 32 Einzel-Bools im Parameterraum, intern Bitmaske; kein
    Bitmasken-Parameter nach außen.
38. **Single-Modus** — `thru.channel` ist der Master-Kanal, `hN.channel` wird
    ignoriert, Werte bleiben gespeichert.
39. **`reset`** — bleibt als Trigger; `stepTrig` entfällt.
40. **ID-Freeze** — Golden File `tests/parameter_ids.txt`, CI bricht bei
    ID-Änderung; Änderung nur mit `schemaVersion`-Erhöhung und Migration.

## Runde 5

41. **JUCE** — 9.0.5 als Submodule auf den Tag-Commit gepinnt; Fallback bei
    Build-Blockern: 8.0.15.
42. **Toolchain** — MSVC 2022 (v143), C++20, CMake ≥ 3.28; Presets `vs2022`
    (MSBuild) und `ninja-ci` (Ninja).
43. **Targets** — `heptachord_core` (ohne JUCE), `heptachord_tests` (Catch2,
    nur Core), `heptachord_plugin`, `heptachord_standalone`. Siehe ADR 0007.
44. **Catch2** — v3 per FetchContent, exakter Tag, `GIT_SHALLOW`.
45. **CI** — GitHub Actions, `windows-latest`, Debug/Release; pluginval mit
    `--strictness-level 5` nur auf Release, blockiert den Merge.
46. **JSON** — nur in der Plugin-Schicht (`juce::var`); Core arbeitet mit einem
    flachen Struct. `schemaVersion` ganzzahlig ab 1, höhere Version = Fehler.
47. **Python-Referenz** — 3.12, nur Standardbibliothek; CSV eingecheckt, CI
    regeneriert und schlägt bei Abweichung fehl.
48. **Release v1** — ZIP mit VST3/Standalone/Presets, keine Signierung,
    SmartScreen-Hinweis in der README.
49. **Versionierung** — SemVer ab `0.1.0` (nach M1), Tags `v0.1.0`; `1.0.0` bei
    v1; `schemaVersion` davon unabhängig.
50. **Doku** — `docs/03-spezifikation.md` als einzige Verhaltensspezifikation;
    der alte Clone-Plan bleibt im Archiv.
51. **M0 Definition of Done** — Repo + LICENSE + README, JUCE gepinnt,
    `parameters.*` vollständig, Generator + Golden File, VST3/Standalone bauen,
    Pluginval Stufe 5 in Reaper, CI grün, ein Commit.
52. **Korrektur zu 31** — Die Parameter-Quelle ist **`parameters.toml`**, nicht
    YAML: `tomllib` ist in der Python-Standardbibliothek, PyYAML wäre eine
    zusätzliche Build-Abhängigkeit. Inhalt und Bedeutung bleiben wie in ADR
    0003 beschrieben.

## Runde 6 (Präzisierung der Spezifikation)

Aus dem Schreiben von `docs/03-spezifikation.md` (Entscheidung 50) ergaben sich
Lücken, die die bisherigen Entscheidungen nicht abdeckten. Sie sind hier
festgehalten; die vollständige Begründung steht in der Spezifikation.

53. **Velocity-Folge** — `hN.follow = true` (Default) heißt: `hN.velocity` wird
    ignoriert, die Stimme spielt die Eingangs-Velocity (mal Spread, mal
    Step-Akzent). `follow = false` heißt: `hN.velocity` wirkt als Faktor auf die
    Eingangs-Velocity. Damit ersetzt `follow` die frühere Konvention
    „`hN.velocity = 0` folgt der Eingabe" (Nr. 9) — 0 % ist jetzt ein
    Sonderfall, der auf 1 geklemmt wird.
54. **Rastung** — `snapToScale` rastet **root-relativ**: die Kandidaten sind
    `root + 12·k + skala[i]`, nicht absolute C-Dur-Töne. Die bisherige
    Implementierung nimmt implizit Root C an und wird in M1 umgestellt.
55. **Thru-Gate** — in v1 gibt es keinen Gate-Parameter für die Thru-Stimme;
    ihr Gate ist fest 100 % der Schrittlänge. Ein `thru.gate` ist Kandidat für
    v1.1 und würde eine `schemaVersion`-Erhöhung brauchen (Nr. 40).
56. **Step-Akzent** — `sN.velocity` wirkt auf **alle** Stimmen des Steps,
    auch auf Thru. `harm.spread` wirkt dagegen nur auf H1–H3.
57. **Ratchet-Verteilung** — Treffer `k` (0-basiert) bei
    `Step-Grenze + k · stepSeconds / ratchet`; die Gate-Länge wird zusätzlich
    auf `stepSeconds / ratchet` geklemmt. Die Wahrscheinlichkeit wird **pro
    Treffer** gewürfelt, nicht pro Step.
58. **Traversal** — Die Richtung läuft über die *aktiven* Steps. PingPong
    wiederholt die Endpunkte nicht (`0 … k−1, k−2 … 1, 0, 1 …`), Zufall darf
    denselben Step zweimal hintereinander wählen. Swing wirkt auf die geraden
    Traversal-Slots (2., 4. …), Slack global auf alle.
59. **Hold** — Der Trigger wird einmal gesetzt; die Note wird über Step-Grenzen
    hinweg gehalten und erst beendet, wenn ein Step kommt, in dem die Stimme
    nicht maskiert ist oder der Modus Play/Ratchet gilt.
60. **Präzisierungen** — Die in der Spezifikation mit `[Präzisierung]`
    markierten Festlegungen gelten als verbindlich, bis sie hier bestätigt oder
    geändert werden.

## Runde 7 (M1 — Step-Engine)

61. **Velocity-Gruppenfaktoren** — `thru.velocity` wirkt auf die Thru-Stimme,
    `harm.velocity` auf H1–H3. Beide bilden die Velocity-CV-Eingänge des
    Originals ab (`CV_In_ThruVel`, `CV_In_HarmVel`). `hN.velocity` ist der
    zusätzliche Stimmentrim und wirkt nur bei `follow = false`. Die Formel in
    `docs/03-spezifikation.md` 6.2 war zuvor unvollständig.
62. **Basisgrad-Nullpunkt** — Der diatonische Grad zählt vom **tiefsten
    Vorkommen des Grundtons** (MIDI `root … root+11`). Der absolute Wert ist
    beliebig, solange die Rechnung konsistent bleibt; festgelegt wurde er
    deshalb, damit die Python-Referenz (Nr. 47) denselben Nullpunkt benutzt.
63. **PingPong** — Die Folge startet beim ersten aktiven Step und wiederholt
    die Endpunkte nicht: `0 … k−1, k−2 … 1, 0, 1 …`. Bei zwei aktiven Steps
    wechselt die Richtung, bei einem bleibt der Cursor stehen.

## Runde 8 (M2a — Ereignisschicht)

64. **Zeitmodell** — Die Slot-Position kommt aus der **Host-ppq**
    (`n = floor(ppq / Slotlänge)`); innerhalb des Blocks wird sie sample-genau
    fortgeschrieben und gegen die Host-ppq geprüft. Weicht der Host um mehr als
    eine Viertel-Slotlänge ab (Locate, Loop-Sprung, Tempoänderung), gilt der
    Host: All-Notes-Off, Traversal beginnt neu beim ersten aktiven Step. Ohne
    Host-Position läuft die Uhr intern weiter. Ein reiner Sample-Zähler oder
    eine blockweise Quantisierung wäre einfacher, würde aber nach einem Locate
    versetzt weiterlaufen bzw. bis zu einer Blocklänge jittern.
65. **Wahrscheinlichkeit pro Stimme und Treffer** — Der Wurf für
    `sN.probability` fällt **pro Stimme und pro Treffer**, nicht einmal pro
    Step. Bei gleicher Wahrscheinlichkeit dürfen also Thru und H1 anschlagen,
    während H2 und H3 ausfallen. Bei 0 % schlägt keine Stimme an, bei 100 %
    jede. Die Gate-Länge ist davon unabhängig.
66. **Ereignisreihenfolge im Block** — Bei gleichem Sample-Offset gilt:
    Gate-Ende (Note-Off), Host-Eingabe, Ratchet-Treffer, Slot-Beginn. Damit
    steht an jeder Step-Grenze das Note-Off vor dem Note-On, und ein
    eintreffendes Note-Off wirkt noch vor dem Trigger dieser Grenze. Gate-Enden
    und laufende Slots werden über die Blockgrenze hinaus weitergereicht, nicht
    abgeschnitten.

## Runde 9 (M2b — Anbindung und Standalone)

67. **`reset` ist flankengetriggert** — Das Plugin löst nur beim Übergang
    0 → 1 aus und schreibt den Parameter nicht zurück. Ein dauerhaft auf 1
    gehaltener Wert (hängender Taster, gehaltene Automation) setzt das Raster
    damit nicht in jedem Block neu. Vorteil gegenüber dem Rückschreiben: die
    Host-Automation bleibt unangetastet.
68. **Kanalwechsel beendet Noten** — Ändert sich die Kanalbelegung
    (`output.mode`, `thru.channel`, `hN.channel`), bekommen die **verlassenen**
    Kanäle sofort ein All-Notes-Off; die Engine setzt zusätzlich ihren
    Stimmenzustand zurück. Ohne das bliebe eine klingende Note auf dem alten
    Kanal hängen, weil das nächste Gate-Ende auf dem neuen Kanal läge.
69. **Test-Synth nur im Standalone** — Der Standalone bekommt einen
    4-stimmigen Sägezahn-Synth (Tiefpass, ADSR) und einen eigenen Transport
    (120 BPM, Leertaste oder Knopf). Der VST3-Ausgang bleibt still, damit die
    Kategorie „Instrument" erhalten bleibt und der Host keine Fremd-Audioquelle
    bekommt. Beide nutzen denselben Core.

## Runde 10 (M3 — Editor)

70. **Einseitiges Layout mit drei Zonen** — Der Editor zeigt alles auf einer
    fixen Seite von 1000×664 logischen Pixeln: oben die globale Leiste, in der
    Mitte die Stimmen (Thru + H1–H3 als Zeilen), unten die acht Step-Spalten.
    Alle 115 Parameter sind gleichzeitig bedienbar, ohne Reiter oder
    ausklappbare Panels aus Nr. 27. Die feste Größe hält das Raster der
    Step-Spalten stabil und macht die Erweiterung nach unten (Nr. 27) für v1
    überflüssig.
71. **Anbindung über `AudioProcessorValueTreeState`** — Jedes Control hängt mit
    einer `SliderAttachment`/`ComboBoxAttachment`/`ButtonAttachment` direkt am
    APVTS; es gibt keinen doppelten Zustand im Editor. Der Reset-Knopf ruft
    `Processor::triggerReset()` auf und setzt keinen Parameter (Nr. 67). Die
    Beschriftungen (Regler-Captions, Zonentitel, Step-Nummern) werden in
    `paint()` aus Anker-Rechtecken gezeichnet, nicht als eigene `Label`-Kinder;
    so bleibt die Kinderzahl bei genau einem Control je Parameter.
72. **Eigenes dunkles Look-and-Feel, eine Akzentfarbe** — Ein eigener
    `LookAndFeel_V4`-Ableger zeichnet Regler, Combos, Toggles und Knöpfe; Farbe
    `#FF8A3D` ist der einzige Akzent (Wert, aktiv, Fokus), dazu drei
    Flächentöne (Hintergrund `#13161A`, Panel `#1B1F25`, erhöht `#232830`) und
    Text `#E8E6E1`. Der echte Lauf zeigte zu kleine Regler: Einzug 7 → 3.5 px,
    Ringbreite 0.16 → 0.15 des Radius, Reglerhöhe 54 px; die Tooltips kommen
    direkt aus `parameters.toml` (`note`-Feld), damit die Doku nicht doppelt
    gepflegt wird.

## Runde 11 (M5 - Presets + QWERTZ-Keyboard)

73. **QWERTZ-Mini-Klaviatur im Standalone** - A-K spielen C bis C einer Oktave
    (weisse Tasten, C-Dur), Y oder Z senkt die Oktave, X hebt sie (klemmt auf
    -2..+2), Leertaste ist der Transport. JUCE liefert auf Windows den
    Zeichencode der Beschriftung, daher funktioniert das Mapping layout-
    unabhaengig: QWERTY und QWERTZ brauchen keine Erkennung. Nur die
    Plugin-Schicht; der Core kennt keinen Unterschied (ADR 0007).
74. **Tastatur-Noten laufen als MIDI in den Eingangspuffer** - Der Editor
    injiziert Note-On/Off in einen mutex-geschuetzten MidiBuffer im Processor;
    processBlock mischt ihn vor der Bridge in midiMessages. Der Guard
    isStandalone() stellt sicher, dass im VST3 nichts ankommt. Note-Off wird
    ueber keyStateChanged + isKeyCurrentlyDown geliefert, Fokusverlust und
    Destruktor schicken alle Noten aus.
75. **Preset-Selektor in der Kopfzeile** - Eine Combo mit den 8 Factory-
    Presets; die Auswahl laedt per setValueNotifyingHost in das APVTS
    (Trigger reset ausgenommen, Nr. 67) und schickt danach All-Notes-Off
    (Spezifikation 8.2/8.3). Kein doppelter State: Quelle bleibt das APVTS.
76. **Weg ueber Normalisierung** - Presets speichern reale Parameterwerte
    (Choice = Index, Bool = 0/1); beim Laden rechnet
    Preset::normalizedValues() in die APVTS-Form 0..1 um. Ein Roundtrip-Test
    sichert Wert -> normalisiert -> Wert ab.
77. **Factory-Preset-Suchpfade** - HEPTACHORD_PRESETS_DIR (Tests), sonst
    Arbeitsverzeichnis/presets, Binary-Verzeichnis/presets und dessen
    Elternverzeichnis/presets (der Standalone liegt in Release/Standalone,
    die Dateien in Release/presets). Schlecht geladene Factory-Presets werden
    still uebersprungen; der Editor zeigt die gefundenen Namen.

## Runde 12 (M6 - MIDI-Clock)

78. **Automatischer erster MIDI-Eingang** - Der Standalone oeffnet beim
    Prepare den ersten verfuegbaren MIDI-Eingang (MidiInput::openDevice) und
    zeigt seinen Status im Editor-Header. Keine Auswahl-Combo (Scope). Kein
    Geraet oder Oeffnen fehlgeschlagen = stiller Fallback auf 120 BPM plus
    manuellem Transport; das Verhalten ohne Clock bleibt identisch zu M5.
    Das VST3 oeffnet keinen Eingang; dort ist der Host-PlayHead die Quelle.
79. **BPM aus 24 Clock-Intervallen** - Ein Viertel = 24 F8; der Tracker
    fuehrt einen Ring der letzten 25 Zeitstempel und rechnet
    bpm = 60 / (t[24] - t[0]), geklemmt auf 30..300 BPM. Die Mittelung
    dampaeft Jitter des Masters; ohne volle 24 Intervalle gilt der Default
    120 BPM. ppq = Clocks/24 seit FA.
80. **Transport und Timeout** - FA startet (ppq = 0), FC stoppt, FB setzt
    ohne Positionsverlust fort; Leertaste/Knopf bleiben zusaetzlich manuell,
    die Clock hat Vorrang, solange sie aktiv ist. 2 s ohne F8 beenden die
    Clock-Quelle: die Position wird einmal uebernommen, danach laeuft der
    Fallback (120 BPM, manueller Transport). Der Timeout verhindert, dass ein
    ausgeschalteter Master das Plugin einfriert.

## Runde 13 (Bugfix Standalone-Klaviatur)

81. **VK-Codes normalisieren** - juce::KeyPress liefert auf Windows bei
    echter Tastatureingabe WM_KEYDOWN-Werte (VK-Codes, 'A'..'Z' = 65..90);
    Zeichencodes ('a'..'z') erreichen keyPressed nur ueber Texteingabe-Pfade,
    da JUCE TranslateMessage nur mit fokussiertem Text-Input-Target aufruft.
    mapKey normalisiert daher VK-Codes auf die Zeichencodes der Tabelle.
    Regressionstest in tests/test_computerkeyboard.cpp. Betroffen war nur
    die Standalone-Mini-Klaviatur (v0.1.0: tot, nur Leertaste wirkte);
    MIDI-Eingang, Presets, Skala und Engine waren korrekt (Ende-zu-Ende
    mit keybd_event-Probe verifiziert).

## Runde 14 (96er-Matrix + Bezeichnungsmodus)

82. **96 Factory-Presets als Grundton-x-Skala-Matrix** - 12 Grundtoene x 8
    Skalen, Slot i = Grundton*8 + Skala; Dateinamen sind nummerierte stabile
    chromatische IDs mit deutschen Buchstaben-Slugs
    (heptachord_89_h_dur.heptachord). Je Skala bleibt das Charakter-Muster
    der bisherigen Presets p1..p8 erhalten, nur der Grundton transponiert.
    Preset::factoryPresets() sortiert Dateien nach Dateinamen
    (findChildFiles garantiert keine Ordnung); der JSON-`name` bleibt die
    deutsche Kanon-Form (Grundton zuerst, z. B. "H Moll"), Anzeigenamen
    leitet der Editor aus root + Skala + names.mode ab. Erzeugung nur ueber
    tools/gen_presets.py. Bugfix dabei entdeckt und gefixt (User-Freigabe
    Runde 87): die Overrides standen als tote Unterstrich-Schluessel
    ("s3_mode") in den Dateien und wurden beim Laden still ignoriert - die
    Charakter-Pattern der 8 Factory-Presets waren nie hoerbar. Der Generator
    schreibt jetzt die echten IDs ("s3.mode"); heptachord_01_c_dur bleibt
    byte-identisch (leeres Muster), 7 C-Dateien aendern Inhalt auf das
    dokumentierte Muster. Tests: Matrix-Struktur, C-gegen-F-Pattern,
    deutsche Kanon-Namen (test_preset.cpp).
83. **names.mode - Bezeichnungsmodus als Anzeige-Option** - Ein Choice-
    Parameter (Index 9, nach `reset` in der Global-Sektion) mit 3 Eintraegen:
    Deutsch (C, Cis, ..., Ais, H; Dur, Dorisch, ...), International (b)
    (C, Db, ..., Bb, B) und International (#) (C, C#, ..., A#, B). Default
    Deutsch. Betrifft nur die Anzeige: root-/scale-Combo-Labels und
    Preset-Anzeigenamen leiten sich ab (src/core/NoteNames.h); die Skalen
    heißen international Major/Minor/Harmonic Minor/Melodic
    Minor/Dorian/Phrygian/Lydian/Mixolydian. Preset-Wechsel setzt den Modus
    nicht zurueck (applyPreset ueberspringt id_names_mode); keine eigene
    Persistenz (Standalone: pro Sitzung, Host-Zustand im VST3 wie ueblich).
    Platzierungs-Hinweis: ein `[[param]]` am toml-Dateiende haette Index 15
    ergeben (Familien expandieren nach den [[param]]-Bloecken); die
    Global-Sektion nach `reset` ist die saubere Stelle - IDs sind
    eingefroren, nur der Index ist neu. Der bisherige 11. Ton "B" wurde zu
    "H" repariert (Mischzustand). Kopfzeilen-Combo "Bezeichnungen" ohne
    Beschriftung (ASCII-Schreibweise b/# in den Eintraegen), Untertitel
    ohne "Stepwise Harmonizer" (Platz), Preset-Combo 200 -> 160 px,
    namesBox 150 px. root/scale-Combos werden bei Moduswechsel per
    ParameterListener + callAsync neu beschriftet; Items-Indices bleiben
    stabil, damit die Attachments weiterbinden.

## Runde 15 (MIDI-Export v0.1.3)

84. **M1 - deterministischer MIDI-Export eines Zyklus** - Der Standalone
    rendert genau einen Zyklus (alle aktiven Steps einmal in
    Traversal-Reihenfolge) ueber dieselbe Planung wie die Live-Engine
    (src/core/MidiRender.cpp nutzt StepPlanner, StepSequencer, applyStep).
    Feste Zufallsquelle (Seed 1) fuer Wahrscheinlichkeit und Richtung
    "Zufall" -> bitgleicher Export bei identischen Parametern (Tests
    test_midirender.cpp/test_midiexport.cpp). Letzte Eingangsnote, sonst
    C4 (MIDI 60) als Fallback. Kein Eingriff in den Audio-Thread, kein
    Host-Transport, kein Wiederholungszaehler (Loopen macht die DAW).
    Core-Render JUCE-frei (ADR 0007); juce::MidiFile nur in der
    Plugin-Schicht (src/plugin/MidiExport.cpp).
85. **M2 - Drag & Drop MIDI** - Der Standalone schreibt eine temporaere
    .mid-Datei (System-Temp/Heptachord/) und startet einen nativen
    External-Drag aus der Steps-Titelzeile ("DRAG"-Knopf). Der Editor ist
    selbst der DragAndDropContainer; der Drag beginnt in mouseDrag nach
    5 px (JUCE 9: startDragging mit Array<var> von Dateipfaden). M3
    (WAV durch den Test-Synth) bleibt spaeterer Bedarf.
86. **Export-Format und Bedienung** - Standard-MIDI Format 1, PPQ 480,
    vier Spuren (0 Thru, 1..3 H1..H3), Tempo-Meta aus der aktuellen
    Standalone-BPM (clockBpm() oder 120 Fallback) plus 4/4-Takt in
    Spur 0. Gate-Enden und Hold-Ketten werden als Note-Offs
    mitgeschrieben; eine ueber das Zyklusende hinaus klingende Note
    endet im File danach. Negativer Slack wird auf Tick 0
    normalisiert. EXPORT- und DRAG-Knopf rechts in der Steps-
    Titelzeile (dort ist Platz; die Kopfzeile ist knapp), nur im
    Standalone - im VST3 zeichnet die DAW auf. Dateiname:
    Heptachord_<Grundton>_<Skala>_<Rate>.mid in ASCII
    (z. B. Heptachord_C_Dur_1-8.mid). JUCE 9: kein modales
    FileChooser/MessageBox (launchAsync/showMessageBoxAsync).

## Runde 16 (WAV-Export M3)

87. **M3 - WAV-Export des Test-Synths** - Der Standalone rendert einen
    Zyklus durch den Test-Synth (Saegezahn + Tiefpass + ADSR) und schreibt
    ein 16-Bit-WAV (src/plugin/AudioExport.cpp). JUCE 9: Stream-Ownership
    durch den AudioFormatWriter (createWriterFor mit unique_ptr<OutputStream>),
    da der deprecated Raw-Pointer-Overload den Stream loescht und einen
    Stack-FileOutputStream doppelt freigeben wuerde. Render ueber einen
    einzigen Block (alle Events mit absoluten Sample-Positionen), 2 Sekunden
    Tail fuer ausklingende Noten. "WAV"-Knopf rechts in der Steps-
    Titelzeile, standalone-only, analog zu EXPORT/DRAG. Dateiname
    Heptachord_<Grundton>_<Skala>_<Rate>.wav (ASCII). Tests in
    tests/test_audioexport.cpp (gueltiges WAV, Peak > 0, leerer Zyklus,
    Dateiname).

## Runde 17 (v0.1.5 - Standalone-Erweiterungen)

88. **Step-Highlight** - Der aktuell abgespielte Step (0..7) wird im
    Standalone durch einen farbigen Rahmen um die entsprechende Step-Spalte
    kenntlich gemacht. Ein Timer mit 30 Hz fragt processor.currentStepIndex()
    ab und löst bei Änderung ein epaint() aus. Der Rahmen nutzt
    HeptachordLookAndFeel::accent() mit 2.0f Linienstärke.
89. **Noten-Export (MIDI + Text)** - Neuer "NOTEN"-Knopf in der Steps-
    Titelzeile (standalone-only, neben EXPORT/DRAG/WAV). Exportiert den
    deterministischen Zyklus als Standard-MIDI (Format 1, PPQ 480, 4 Spuren,
    Tempo-Meta) sowie als Text/CSV mit Spalten Step, Tick, Kanal, Pitch,
    Velocity, Event (On/Off). Dateiname Heptachord_Noten_<Zeitstempel>.mid
    bzw. .txt auf dem Desktop.
90. **Tempo-Parameter** - Neuer globaler Parameter 	empo (float, 30.0..300.0,
    Default 120.0, Einheit "BPM") ersetzt den fest verdrahteten
    kStandaloneBpm = 120.0. Wird in uildTimeline() und exportOptions()
    verwendet, wenn kein MIDI-Clock aktiv ist. Parameter-Tabelle von 117 auf
    118 Parameter erweitert via 	ools/gen_params.py.
91. **VST-Instrument-Hosting (Standalone)** - Neuer Instrument-Interface mit
    zwei Adaptern: TestSynthInstrument (eingebauter Synth, Default) und
    VstPluginInstrument (lädt eine VST3-Datei per AudioPluginFormatManager).
    Der Processor hält ein std::unique_ptr<Instrument> und ruft dessen
    process() im processBlock auf statt direkt m_synth. UI mit
    instrumentBox (ComboBox), loadButton ("LADEN...") und
    editButton ("EDITOR") im Header-Bereich. Nur im Standalone, nicht im
    VST3-Build. CMake verlinkt juce::juce_audio_plugin_client.

## Runde 18 (v0.1.6 - Standalone-Verbesserungen)

92. **Step-Highlight sichtbar** - Gefuellter Hintergrund der aktiven Step-Spalte
    (panelRaised, 0xff232830) + linker Akzentrand (accent, 3px Breite) +
    Playhead-Dreieck oben an der Spalte. Ersetzt den zuvor unsichtbaren duennen
    Rahmen. Timer mit 30 Hz fragt processor.currentStepIndex() ab.
93. **INSTRUMENT-Panel im Global-Bereich** - instrumentBox, loadButton, editButton
    aus dem ueberfuellten Header entfernt, stattdessen eigenes INSTRUMENT-Panel
    als 12. Zelle im Global-Bereich (rechts von Output). Prominenter und
    uebersichtlicher.
94. **GUI-Zoom** - Feste Stufen 100/125/150/175/200 % ueber ComboBox im Header.
    setTransform(AffineTransform::scale(zoom, zoom)) am Ende von resized().
    Gilt fuer Standalone und VST3.
95. **Kein Reset bei Tonart/Skala-Aenderung** - Bereits korrekt implementiert:
    reset-Trigger ist separater Parameter (Flanke 0->1), setSettings() uebernimmt
    neue Werte ohne Cursor-Reset. applyLayoutChange() beendet nur Noten auf
    Kanalen, die wegfallen.
