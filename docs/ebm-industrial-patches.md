# EBM / Industrial / Industrial Metal / Industrial Rock – Patch-Notizen

> **INFORMELLE NOTIZ – KEIN RELEASE-DOKUMENT.**
> Dieses Dokument ist **nicht** Teil der Heptachord-Produkt-/Release-Dokumentation, sondern eine private, informelle Sammlung zum Thema Synth-Presets für EBM/Industrial und Gesaffelstein-Ähnlichkeit. Es beschreibt ausschließlich Presets, die auf diesem Rechner installiert sind, und enthält keine Aussage über Heptachord-Funktionen, -Qualität oder -Support.

**Stand:** 2026-11-19 · **Erstellt durch:** Recherche-Assistent · **Sprache:** Deutsch

## 1. Wie diese Liste entstanden ist – und was sie *nicht* ist

- **Ground Truth sind die Dateinamen auf der Festplatte.** Alle unten genannten Patch-Namen wurden per Dateisystem-Inventur (siehe [Quellen](#6-quellen)) gefunden und mit einer automatischen Namensprüfung gegen die Verzeichnisse verifiziert. Es wurde **kein** Patch-Name erfunden, übersetzt oder „verschönert“ – auch nicht dort, wo die Namensgebung ungewöhnlich ist (z. B. `Harsch Bass`, `Brutal Scream Bas`).
- **Es wurde nicht gehört.** Es gab kein Audio-Auditioning und keine Parameteranalyse der Presets. Die Aussagen zum Klangcharakter sind daher **vermutete, begründete Hinweise aus Name + Kategorieordner**, keine gehörten Tatsachen.
- **Konfidenz-Legende:**
  - 🟢 **Name/Typ weist klar hin** – der Patch-Name oder der Ordner nennt das Ziel direkt (z. B. `EBMania`, `Industrial Bass`, `EBMLead`, Ordner `Bass`).
  - 🟡 **Worth auditioning** – plausibel aus Name/Genre-Assoziation, aber ohne Gehör bestätigungsbedürftig.
- Die Web-Recherche liefert **nur Genre-Kontext**. Daraus abgeleitete Patch-Zuordnungen sind ausdrücklich **reasoned suggestions** (begründete Vermutungen), keine Fakten über einen konkreten Patch.

### Genre-Kontext (aus Webquellen, siehe [Quellen](#6-quellen))

- EBM: repetitive, sequenzierte Basslines, programmierte 4/4-Beats, Echo/Hall, gelegentlich Maschinen-/ Alarmgeräusche; prägende Geräte waren u. a. Korg MS-20, Roland SH-101, Oberheim und Yamaha DX7 ([Wikipedia: Electronic body music](https://en.wikipedia.org/wiki/Electronic_body_music), [Darkwaveradio: EBM](https://darkwaveradio.net/electronic-body-music-ebm/), [Front 242 – About](https://www.front242.com/about.html), [Electronic Body Music – Reading sample, PDF](https://www.elektronische-koerpermusik.de/download/Electronic%20Body%20Music%20-%20Reading%20sample.pdf)).
- Industrial / Industrial Rock / Industrial Metal: harte, maschinelle, repetitive Texturen, verzerrte Gitarren-Riffs bzw. Samples, Synthesizer-/Sequencer-Linien, verzerrte Vocals, metallische Perkussion, Sirenen ([Wikipedia: Industrial metal](https://en.wikipedia.org/wiki/Industrial_metal), [Wikipedia: Industrial rock](https://en.wikipedia.org/wiki/Industrial_rock), [AllMusic: Industrial](https://www.allmusic.com/genre/industrial-ma0000002658/albums), [Tagg/Collins: The Sonic Aesthetics of the Industrial, PDF](https://tagg.org/articles/xpdfs/dartington2001.pdf)).
- Für Industrial **Metal / Rock** ist entscheidend, dass die Synth-Patches hier eher **Gitaren- speaking** (`Distorted Guitar`, `Guitar for Robots`, `Metallic Bass`) und **harte Transienten** liefern; verzerrte Gitarre selbst kommt aus einem Gitarren-Amp/Modellierer, nicht aus diesen Presets.

---

## 2. Patch-Vorschläge nach Rolle

Hinweis zu den Pfaden: `OB-Xd 3` = `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets`, `DX7 V` = `C:\ProgramData\Arturia\Presets\DX7 V\Factory`, `AL3` = `C:\ProgramData\Arturia\Presets\Analog Lab 3\Factory`. Die Pfade in der Tabelle sind trotzdem vollständig angegeben, damit sie direkt zum Kopieren geeignet sind.

### 2.1 BASS

| Patch (exakt) | Synth | Pfad | Warum (begründete Vermutung) | Konfidenz |
|---|---|---|---|---|
| `LostEBMBass` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Bass\LostEBMBass.fxp` | Name nennt EBM direkt; liegt im Ordner `Bass`. Analoger Oberheim-Character passt zur klassischen EBM-Basslinie. | 🟢 |
| `RawEBMBass` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Bass\RawEBMBass.fxp` | „Raw“ + EBM: naheliegender Kandidat für einen ungeschliffenen, aggressiven Sequencer-Bass. | 🟢 |
| `Brutal Scream Bas` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Bass\Brutal Scream Bas.fxp` | Name deutet auf schreienden, extremen Bass (Dateiname ist im Original abgeschnitten: „Bas“). | 🟢 |
| `Elektro Bass 04` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Bass\Elektro Bass 04.fxp` | Teil einer Elektro-Bass-Serie; geeignet für harte, rhythmische Elektro-/EBM-Linien. | 🟡 |
| `Sync Bass` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Bass\Sync Bass.fxp` | Sync-Klang ist ein klassisches Merkmal aggressiver Industrial-/Electro-Bässe. | 🟢 |
| `Hard Attack` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Bass\Hard Attack.fxp` | Name verspricht harte Transienten – nützlich für „pile-driver“-Rhythmen. | 🟢 |
| `EBMania` | DX7 V | `C:\ProgramData\Arturia\Presets\DX7 V\Factory\Factory\EBMania` | Name ist ein direkter EBM-Hinweis; DX7-FM liefert die kühle, metallische Seite des Genres. | 🟢 |
| `Metallic Bass` | DX7 V | `C:\ProgramData\Arturia\Presets\DX7 V\Factory\Factory\Metallic Bass` | Metallischer FM-Bass: naheliegend für Industrial Metal / kalte EBM. | 🟢 |
| `Crawling Bass` | DX7 V | `C:\ProgramData\Arturia\Presets\DX7 V\Factory\Factory\Crawling Bass` | „Kriechend“ deutet auf dunkle, langsame Motion – gut für depressive Dark-Electro-Passagen. | 🟡 |
| `Electric Saw Bass` | DX7 V | `C:\ProgramData\Arturia\Presets\DX7 V\Factory\Factory\Electric Saw Bass` | Sägezahn-Grundklang ist die typische Basis für verzerrbare Industrial-Bässe. | 🟢 |
| `Industrial Bass` | Analog Lab 3 | `C:\ProgramData\Arturia\Presets\Analog Lab 3\Factory\Factory\Industrial Bass` | Name nennt das Zielgenre wörtlich; bester erster Anlaufpunkt im Analog-Lab-Set. | 🟢 |
| `Harsch Bass` | Analog Lab 3 | `C:\ProgramData\Arturia\Presets\Analog Lab 3\Factory\Factory\Harsch Bass` | Offenbar „harsh“ (Originalschreibweise so übernommen): vielversprechend für harte Verzerrung. | 🟢 |
| `Obey The Computer` | Analog Lab 3 | `C:\ProgramData\Arturia\Presets\Analog Lab 3\Factory\Factory\Obey The Computer` | Name evoziert Maschine/Automation – passt zur EBM-Ästhetik, Klang muss geprüft werden. | 🟡 |
| `5x7x303` | Analog Lab 3 | `C:\ProgramData\Arturia\Presets\Analog Lab 3\Factory\Factory\5x7x303` | 303-Assoziation: Säure-/Sequencer-Bass, sehr wahrscheinlich tauglich für EBM. | 🟢 |
| `Pentatonic Acid` | Analog Lab 3 | `C:\ProgramData\Arturia\Presets\Analog Lab 3\Factory\Factory\Pentatonic Acid` | „Acid“ + pentatonisch: repetitive, schneidende Linien, gut für tanzbare EBM-Accents. | 🟢 |

### 2.2 LEAD

| Patch (exakt) | Synth | Pfad | Warum (begründete Vermutung) | Konfidenz |
|---|---|---|---|---|
| `EBMLead` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Lead Synth\EBMLead.fxp` | Name nennt EBM direkt, liegt im Ordner `Lead Synth`. | 🟢 |
| `Prodigy Sync` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Lead Synth\Prodigy Sync.fxp` | Sync-Lead mit Name, der auf aggressiven Rave-/Industrial-Rock-Lead hindeutet (Assoziation, keine Klangaussage). | 🟡 |
| `Sync Beast` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Lead Synth\Sync Beast.fxp` | „Sync“ + „Beast“: aggressiver, schreiender Lead wahrscheinlich. | 🟢 |
| `Razor Lead` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Lead Synth\Razor Lead.fxp` | Name deutet auf scharfe, schneidende Lead-Farben. | 🟢 |
| `Noisy Lead` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Lead Synth\Noisy Lead.fxp` | Rauschanteil ist ein Kernmittel industrieller Leads. | 🟢 |
| `Depeche Lead` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Lead Synth\Depeche Lead.fxp` | Name verweist auf dunklen Synth-Pop; EBM und Dark-Synthpop sind verwandt (siehe Guardian-Quelle). | 🟡 |
| `Dark Psy Lead` | DX7 V | `C:\ProgramData\Arturia\Presets\DX7 V\Factory\Factory\Dark Psy Lead` | „Dark“ + FM: naheliegend für kühle, bedrohliche Leads. | 🟢 |
| `Dist Lead` | DX7 V | `C:\ProgramData\Arturia\Presets\DX7 V\Factory\Factory\Dist Lead` | Verzerrter Lead – direkt passend zu Industrial Rock/Metal. | 🟢 |
| `DistorLead` | DX7 V | `C:\ProgramData\Arturia\Presets\DX7 V\Factory\Factory\DistorLead` | Ebenfalls explizit verzerrter Lead; mit `Dist Lead` vergleichen. | 🟢 |
| `Agro Seeker` | DX7 V | `C:\ProgramData\Arturia\Presets\DX7 V\Factory\Factory\Agro Seeker` | „Agro“ = aggressiv; plausibel für harte Leads, aber gehörbestätigungsbedürftig. | 🟡 |
| `Bite This` | DX7 V | `C:\ProgramData\Arturia\Presets\DX7 V\Factory\Factory\Bite This` | Bissiger Name; kann als schneidende Lead-/Stab-Textur funktionieren. | 🟡 |
| `Guitarps Collide` | Analog Lab 3 | `C:\ProgramData\Arturia\Presets\Analog Lab 3\Factory\Factory\Guitarps Collide` | Name legt einen Gitarren-/Synth-Zusammenstoß nahe – interessant für Industrial Rock/Metal. | 🟡 |

### 2.3 PAD / ATMOS

| Patch (exakt) | Synth | Pfad | Warum (begründete Vermutung) | Konfidenz |
|---|---|---|---|---|
| `Cold Pad` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Pads And Strings\Cold Pad.fxp` | „Kalt“ ist ein direkter Hinweis auf die kühle, minimalistische Pad-Seite (auch gesaffelstein-nah). | 🟢 |
| `Terror OB-Xa` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Pads And Strings\Terror OB-Xa.fxp` | Name deutet auf unheilvolle, dunkle Flächen. | 🟢 |
| `Phobos` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Pads And Strings\Phobos.fxp` | Mond-/Angst-Assoziation: eher Atmosphäre als Melodie. | 🟡 |
| `Bright Drone OB-Xa` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Pads And Strings\Bright Drone OB-Xa.fxp` | Drone = statische, dumpfe Basis für industrielle Spannungsbögen. | 🟢 |
| `Filter Drone OB-Xa` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Pads And Strings\Filter Drone OB-Xa.fxp` | Filterbewegungen auf Drones sind ein typisches Industrial-/Dark-Techno-Mittel. | 🟢 |
| `Haunting Voices` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Pads And Strings\Haunting Voices.fxp` | Unheimliche Stimmen-/Chor-Textur; passt zur „factory ambiance“/-Horror-Seite. | 🟡 |
| `Dark And Space` | Analog Lab 3 | `C:\ProgramData\Arturia\Presets\Analog Lab 3\Factory\Factory\Dark And Space` | Name ist ein direkter Genre-/Stimmungs-Treffer. | 🟢 |
| `Necromorph Amb` | Analog Lab 3 | `C:\ProgramData\Arturia\Presets\Analog Lab 3\Factory\Factory\Necromorph Amb` | „Amb“ + Horror-Association: dunkle Ambient-Fläche. | 🟢 |
| `Phrygian Drone` | Analog Lab 3 | `C:\ProgramData\Arturia\Presets\Analog Lab 3\Factory\Factory\Phrygian Drone` | Phrygisch gilt als „dunkle“ Tonart; Drone passt zu minimalistischer Kälte. | 🟢 |
| `Space Mass` | Analog Lab 3 | `C:\ProgramData\Arturia\Presets\Analog Lab 3\Factory\Factory\Space Mass` | Name deutet auf große, dichte Flächen – gut für wuchtige Industrial-Breaks. | 🟡 |
| `Genesis Graveyard` | DX7 V | `C:\ProgramData\Arturia\Presets\DX7 V\Factory\Factory\Genesis Graveyard` | FM-Kälte + Friedhofs-Assoziation: gute Basis für kalte, klingende Pads. | 🟡 |
| `Digital Graveyard` | DX7 V | `C:\ProgramData\Arturia\Presets\DX7 V\Factory\Factory\Digital Graveyard` | Digital-kühle Variante derselben Idee; mit `Genesis Graveyard` vergleichen. | 🟡 |
| `Glitch Textured Pad` | DX7 V | `C:\ProgramData\Arturia\Presets\DX7 V\Factory\Factory\Glitch Textured Pad` | Glitch + Textur: naheliegend für industrielle Fehler-/Artefakt-Flächen. | 🟢 |
| `Noise Pad` | DX7 V | `C:\ProgramData\Arturia\Presets\DX7 V\Factory\Factory\Noise Pad` | Rauschfläche = direkter Industrial-/Ambient-Baustein. | 🟢 |
| `Premonition Cluster` | DX7 V | `C:\ProgramData\Arturia\Presets\DX7 V\Factory\Factory\Premonition Cluster` | Cluster = dichte, nicht auflösende Klangmasse; unheilvolle Atmosphäre wahrscheinlich. | 🟡 |

### 2.4 ARP / SEQ

| Patch (exakt) | Synth | Pfad | Warum (begründete Vermutung) | Konfidenz |
|---|---|---|---|---|
| `Cylons` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Arp\Cylons.fxp` | Maschinen-/Roboter-Assoziation, liegt im Ordner `Arp`. | 🟡 |
| `PWM Gater` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Arp\PWM Gater.fxp` | Gating ist ein Kernmittel des EBM-/Industrial-Sequencer-Sounds. | 🟢 |
| `Pseudo Seq` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Arp\Pseudo Seq.fxp` | Name sagt Sequencer; gute Basis für wiederholte 16tel-Muster. | 🟢 |
| `SciFi S&H` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Arp\SciFi S&H.fxp` | Sample & Hold erzeugt zufällige, maschinelle Tonfolgen. | 🟢 |
| `Droid under water` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Arp\Droid under water.fxp` | Roboter-Assoziation + Arp-Ordner: industriale Textur wahrscheinlich. | 🟡 |
| `S-H Port Rezz OB-Xa` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Arp\S-H Port Rezz OB-Xa.fxp` | Sample & Hold + Portamento + Resonanz: klassische EBM-Sequencer-Zutaten. | 🟢 |
| `Acid Bass Sequence` | DX7 V | `C:\ProgramData\Arturia\Presets\DX7 V\Factory\Factory\Acid Bass Sequence` | Name nennt Sequenz und Säure; auch als Bass einsetzbar. | 🟢 |
| `Sequencer Bass` | DX7 V | `C:\ProgramData\Arturia\Presets\DX7 V\Factory\Factory\Sequencer Bass` | Direkter Sequencer-Hinweis. | 🟢 |
| `Slick City Seq` | DX7 V | `C:\ProgramData\Arturia\Presets\DX7 V\Factory\Factory\Slick City Seq` | Sequenz-Patch mit urbanem Namen; für rhythmische Pattern geeignet. | 🟡 |
| `Steel Arp` | DX7 V | `C:\ProgramData\Arturia\Presets\DX7 V\Factory\Factory\Steel Arp` | „Stahl“ + Arp: metallische Arpeggios, passend zu Industrial. | 🟢 |
| `Glitch Seq` | DX7 V | `C:\ProgramData\Arturia\Presets\DX7 V\Factory\R.Devine's Signature\Glitch Seq` | Glitch-Sequencer: Fehlerästhetik ist ein Kern der Industrial-Produktion. | 🟢 |
| `Werk Sequences` | Analog Lab 3 | `C:\ProgramData\Arturia\Presets\Analog Lab 3\Factory\Factory\Werk Sequences` | „Werk“ (Arbeit/Werkstatt) + Sequences: der Name ist ein direkter Industrial-Treffer. | 🟢 |
| `Dual Retro Sequences` | Analog Lab 3 | `C:\ProgramData\Arturia\Presets\Analog Lab 3\Factory\Factory\Dual Retro Sequences` | Zwei parallele Seq-Linien ergeben dichte, repetitive Muster. | 🟡 |
| `BR Endo Sequence` | Analog Lab 3 | `C:\ProgramData\Arturia\Presets\Analog Lab 3\Factory\Factory\BR Endo Sequence` | Sequenz mit Film-/Blade-Runner-Assoziation (BR); kalt-cinematische Industrial-Textur wahrscheinlich. | 🟡 |
| `Ambient Seq Split` | Analog Lab 3 | `C:\ProgramData\Arturia\Presets\Analog Lab 3\Factory\Factory\Ambient Seq Split` | Seq plus Ambient-Anteil: gut für die ruhigen, bedrohlichen Zwischenteile. | 🟡 |

### 2.5 FX / PERCUSSION

| Patch (exakt) | Synth | Pfad | Warum (begründete Vermutung) | Konfidenz |
|---|---|---|---|---|
| `Bowed Metal` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Effects\Bowed Metal.fxp` | „Gestrichenes Metall“: metallische Reibegeräusche, wie sie in Industrial-Samples vorkommen. | 🟢 |
| `SYN Wall of Rez` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Effects\SYN Wall of Rez.fxp` | Resonanzwand = intensive Filter-/Feedback-Textur. | 🟢 |
| `Chopper OB-Xa` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Effects\Chopper OB-Xa.fxp` | „Chopper“ deutet auf rotor-/gate-artige Modulation. | 🟢 |
| `Siren FX` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Effects\Siren FX.fxp` | Sirenen werden in EBM/Industrial als Alarm-Sample eingesetzt (siehe Tagg/Collins). | 🟢 |
| `Modulated Riser` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Effects\Modulated Riser.fxp` | Riser für Spannungsaufbau vor dem Drop/Break. | 🟢 |
| `Riser FX 4` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Effects\Riser FX 4.fxp` | Teil einer Riser-Serie (`Riser FX 3`–`6` vorhanden), zum Durchprobieren. | 🟡 |
| `Percussion OB-Xa` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Drums & Percs\Percussion OB-Xa.fxp` | Synthetische Perkussion aus dem Ordner `Drums & Percs`. | 🟢 |
| `NoiKickXL` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Drums & Percs\NoiKickXL.fxp` | Noise-Kick: hart, industrial, gut für den „pile-driver“-Beat. | 🟢 |
| `DaftClaps (C2-C4)` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Drums & Percs\DaftClaps (C2-C4).fxp` | Clap-Sounds über einen Tastenbereich (C2–C4) gelegt. | 🟡 |
| `Synth Drums OB-Xd` | OB-Xd 3 | `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets\Drums & Percs\Synth Drums OB-Xd.fxp` | Komplettes synthetisches Drum-Set, passend zum analogen EBM-Klang. | 🟢 |
| `Very Broken Machinery` | DX7 V | `C:\ProgramData\Arturia\Presets\DX7 V\Factory\Factory\Very Broken Machinery` | Name ist ein direkter Industrial-Treffer: kaputt klingende Maschinerie. | 🟢 |
| `Digi Dirt Beep` | DX7 V | `C:\ProgramData\Arturia\Presets\DX7 V\Factory\Factory\Digi Dirt Beep` | Schmutzige digitale Pieps-SFX – gut für Computer-/Glitch-Texturen. | 🟢 |
| `Perc Radio Noise` | DX7 V | `C:\ProgramData\Arturia\Presets\DX7 V\Factory\Factory\Perc Radio Noise` | Perkussives Radio-Rauschen: klassisches Industrial-FX-Material. | 🟢 |
| `Guitar for Robots` | DX7 V | `C:\ProgramData\Arturia\Presets\DX7 V\Factory\Factory\Guitar for Robots` | Gitarren-Imitat für Roboter – naheliegend für Industrial Metal/Rock. | 🟡 |
| `Ominous Risers` | DX7 V | `C:\ProgramData\Arturia\Presets\DX7 V\Factory\Factory\Ominous Risers` | Unheilvolle Riser; düstere Übergänge. | 🟢 |
| `Electro Riser` | Analog Lab 3 | `C:\ProgramData\Arturia\Presets\Analog Lab 3\Factory\Factory\Electro Riser` | Electro-Riser für den Übergang in den Sequencer-Teil. | 🟢 |
| `Scuba Hit` | Analog Lab 3 | `C:\ProgramData\Arturia\Presets\Analog Lab 3\Factory\Factory\Scuba Hit` | Name deutet auf einen kurzen, harten Hit/Impact. | 🟡 |

---

## 3. Gesaffelstein-Ähnlichkeit

**Was die Referenz ausmacht (Webquellen, keine Patch-Aussage):** Die journalistische Beschreibung von *Aleph* lautet „propulsively minimalist underworld characterized by violence, addiction, paranoia, and rage“ ([Pitchfork, Interview 2014](https://pitchfork.com/features/interview/9322-gesaffelstein/)); er selbst beschreibt die Musik als „really aggressive and really dark“ ([Vice/Noisey, 2014](https://www.vice.com/en/article/if-i-use-a-dark-thing-to-make-people-happy-i-win-gesaffelstein-opens-up/)). Als Einflüsse werden u. a. DAF, Nitzer Ebb, der Korg MS-20, Depeche Mode, Detroit Electro und EBM genannt ([Les Inrocks, Interview 2013](https://www.lesinrocks.com/musique/aleph-de-gesaffelstein-de-la-techno-de-science-fiction-56217-13-11-2013/), [Mixmag](https://mixmag.net/feature/everything-need-to-know-gesaffelstein), [Gearnews](https://www.gearnews.com/gesaffelstein/)). Pitchfork hebt „analog-rooted sound design“, „glassy FM tones suggesting the steely clank of chains“ und aufbauende Verzerrung hervor – allerdings im Hinblick auf das spätere, deutlich poppigere Album *GAMMA* ([Pitchfork, GAMMA-Review 2024](https://pitchfork.com/reviews/albums/gesaffelstein-gamma/)); als Referenz für den „düsteren“ Gesaffelstein gilt eher die *Aleph*-Ära.

**Daraus abgeleitete, begründete Startpunkte (keine Emulation, kein Gehört-Urteil):**

| Klangziel | Nächste lokale Startpunkte | Warum (reasoned suggestion) |
|---|---|---|
| Verzerrter, dunkler Analog-Bass | `LostEBMBass`, `RawEBMBass`, `Brutal Scream Bas` (OB-Xd 3); `Harsch Bass`, `Industrial Bass` (AL3) | Namen deuten auf rohe, harte Bässe; Oberheim-/Analoge Modelle sind die passende Grundlage. |
| FM / metallische, kalte Leads | `EBMania`, `Metallic Bass`, `Dark Psy Lead`, `Dist Lead` (DX7 V); `EBMLead`, `Sync Beast` (OB-Xd 3) | FM liefert die „stählernen“ Obertöne; Sync/Verzerrung die Aggressivität. |
| Kalte, minimalistische Pads | `Cold Pad`, `Terror OB-Xa` (OB-Xd 3); `Dark And Space`, `Phrygian Drone` (AL3); `Glitch Textured Pad` (DX7 V) | Namen adressieren Kälte, Düsternis, Drone und Textur. |
| Harte industrielle Texturen / FX | `Very Broken Machinery`, `Digi Dirt Beep` (DX7 V); `Bowed Metal`, `SYN Wall of Rez` (OB-Xd 3) | Metall-, Bruch- und Resonanztexturen entsprechen der beschriebenen Ästhetik. |

**Wichtige Einschränkung:** Eine exakte Emulation entsteht nicht durch das Laden eines Presets. Verzerrung, Filterfahrten, Kompression, Sidechain, präzises Sequencing (Syncopation, Pausen) und der Mix sind entscheidend. Auch der vorhandene User-Preset `Gesaffelstein - Orck Bass` (`C:\Users\zichosch.thomas\Documents\Vital\User\Presets\Gesaffelstein - Orck Bass.vital`) und `GesaffelsteinLike.vital` im selben Ordner sind **Namedrop-Presets eines Dritten**, keine Garantie für Nähe zum Original – und sie gehören nicht zu den drei hier fokussierten Synths (siehe Bonus unten).

**Bonus außerhalb des Fokus (ebenfalls echte Dateien, nur der Vollständigkeit halber):**
- `Gesaffelstein - Orck Bass` – `C:\Users\zichosch.thomas\Documents\Vital\User\Presets\Gesaffelstein - Orck Bass.vital`
- `GesaffelsteinLike.vital` – `C:\Users\zichosch.thomas\Documents\Vital\User\Presets\GesaffelsteinLike.vital`
- `Dispersed Grit` – `C:\Users\zichosch.thomas\Documents\Vital\Navi Retlav Studio\Presets\Factory Presets\Dispersed Grit.vital`
- `Distorted Bass`, `Distorted FM`, `Doomsday`, `Metal Square` – z. B. `C:\Program Files\Common Files\VST3\Surge Synth Team\SurgeXTData\patches_factory\` (Surge XT Factory, `.fxp`)

---

## 4. So testest du das mit Heptachord

Faktisch: Bei allen oben genannten Einträgen handelt es sich um **Performance-Presets innerhalb der jeweiligen VST-Plugins** (OB-Xd 3, Arturia DX7 V, Arturia Analog Lab 3). Der Ablauf ist daher:

1. In Heptachord ein Instrumenten-Track mit dem gewünschten VST (OB-Xd 3, DX7 V oder Analog Lab 3) anlegen/auswählen.
2. Im Plugin den Patch **über den exakten Namen** laden (OB-Xd: `.fxp`-Presets u. a. aus den Ordnern `Bass`, `Lead Synth`, `Arp`, `Effects`, `Drums & Percs`; Arturia: Presets im Plugin-Browser, z. B. `Factory\Factory`).
3. Noten spielen und/oder aufnehmen; die Klangbewertung erfolgt im Gehör, nicht anhand dieser Liste.
4. **An Heptachord selbst sind keine Änderungen nötig** – die Patches liegen bereits lokal und werden vom VST geladen. Diese Notiz ist kein Hinweis auf ein Heptachord-Feature und kein Support-Dokument.

---

## 5. Ehrlichkeits-/Grenzen-Vermerk

- Analog Lab 3 und DX7 V speichern Presets hier als **Dateien ohne Dateiendung** (`Industrial Bass`, `EBMania`, …). Die Patch-Namen wurden aus den Dateinamen übernommen, nicht aus XML-Metadaten oder aus dem Plugin ausgelesen.
- OB-Xd 3-Presets sind `.fxp`-Dateien; die Namen wurden aus den Dateinamen (ohne `.fxp`) übernommen.
- Ein zweiter, älterer Ordner `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd\Presets` existiert mit denselben Kategorien (u. a. `LostEBMBass.fxp`, `RawEBMBass.fxp`, `EBMLead.fxp`). Die Hauptliste bezieht sich bewusst auf **OB-Xd 3**.
- Es wurde **kein** Patch angehört, **kein** Parameterwert geprüft und **keine** Klanggleichheit mit einem Künstler behauptet. Alle Zuordnungen zu Genres oder zu Gesaffelstein sind begründete Vermutungen.
- Webaussagen sind ausschließlich Genre-/Künstler-Kontext und wurden nicht als Fakten über lokale Presets verwendet.

---

## 6. Quellen

### Lokal (Ground Truth für alle Patch-Namen)

- `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd 3\Presets` – Hauptquelle (Ordner `Arp`, `Bass`, `Drums & Percs`, `Effects`, `Keys`, `Lead Synth`, `Pads And Strings`, `Plucked`, `Synth Poly`; 3 Root-`.fxp` + ca. 1002 `.fxp` in Unterordnern).
- `C:\Users\zichosch.thomas\Documents\discoDSP\OB-Xd\Presets` – älterer OB-Xd-Presetordner (zum Vergleich).
- `C:\ProgramData\Arturia\Presets\DX7 V\Factory\Factory` – DX7-V-Factory-Presets (Namensdateien ohne Endung); zusätzlich `Factory\Lyli's Signature`, `Factory\R.Devine's Signature`.
- `C:\ProgramData\Arturia\Presets\Analog Lab 3\Factory\Factory` – Analog-Lab-3-Factory-Presets (Namensdateien ohne Endung); zusätzlich `Factory\Vangelis Tribute` (dort u. a. `Dervish D. Seq & Lead`).
- `C:\ProgramData\Arturia\DX7 V\resources` und `C:\ProgramData\Arturia\Analog Lab 3\resources` – Plugin-Ressourcen (XML/GUI/Algorithmen), **keine** Patch-Namensquelle.
- `C:\Users\zichosch.thomas\Documents\Vital\User\Presets`, `C:\Program Files\Common Files\VST3\Surge Synth Team\SurgeXTData\patches_factory` – nur für den Bonus-Abschnitt.

### Web (nur Genre-Kontext, mit Behauptung pro URL)

- https://en.wikipedia.org/wiki/Electronic_body_music — EBM: sequenzierte Basslines, programmierte Beats, Hall/Echo, Umgebungssamples; typische Geräte u. a. MS-20, DX7, Oberheim.
- https://darkwaveradio.net/electronic-body-music-ebm/ — EBM: harte tanzbare Beats, repetitive Bass-Sequencer-Linien.
- https://www.front242.com/about.html — offizielle Bandseite: EBM als „collage and synthesis of music and sound“, Media-/TV-Samples, 1984 EP *No Comment*.
- https://www.elektronische-koerpermusik.de/download/Electronic%20Body%20Music%20-%20Reading%20sample.pdf — DAF/Die Krupps: pochende Rhythmen, Step-Sequencer-Basslines, Minimalismus.
- https://www.theguardian.com/music/2024/dec/17/music-80s-ebm-electronic-body-music — EBM als „tough, muscular, sexual, intensely physical“; „Like Depeche Mode, but evil“.
- https://djmag.com/content/unstoppable-influence-ebm — pulsierende Basslines, gutturale Synths, „war-like noises“.
- https://en.wikipedia.org/wiki/Industrial_metal — Fusion aus Metal und Industrial: wiederholte Metal-Riffs, Sampling, Synth-/Sequencer-Linien, verzerrte Vocals.
- https://en.wikipedia.org/wiki/Industrial_rock — Glasgow/Chicago-Linie; harte, schrille Sounds, verzerrte Gitarrenriffs plus elektronische Texturen.
- https://www.allmusic.com/genre/industrial-ma0000002658/albums — Industrial als harsche, mechanisch-repeatende Fusion; EBM = „pummeling dance beats“.
- https://tagg.org/articles/xpdfs/dartington2001.pdf — maschinelle/industrielle Samples, Powerchords „wie eine Maschine“ gespielt, verzerrte eintönige Vocals, Sirenen.
- https://www.masterclass.com/articles/industrial-music-guide — Industrial mischt Rock-Instrumente mit Synths/Sequencern; EBM betont tanzbare Beats.
- https://pitchfork.com/features/interview/9322-gesaffelstein/ — *Aleph* als minimalistische, gewaltvolle Underground-Techno-Ästhetik; Kanye-West-Kollaboration.
- https://www.vice.com/en/article/if-i-use-a-dark-thing-to-make-people-happy-i-win-gesaffelstein-opens-up/ — Gesaffelstein über „really aggressive and really dark“.
- https://www.lesinrocks.com/musique/aleph-de-gesaffelstein-de-la-techno-de-science-fiction-56217-13-11-2013/ — Interview: DAF/Nitzer-Ebb-Einflüsse, Korg MS-20, Molltonarten, Dunkelheit.
- https://mixmag.net/feature/everything-need-to-know-gesaffelstein — frühe Einflüsse Depeche Mode, Detroit Electro, EBM.
- https://www.gearnews.com/gesaffelstein/ — „dark and minimalist“; MS-20, MiniBrute, MFB Synth II als Werkzeuge.
- https://pitchfork.com/reviews/albums/gesaffelstein-gamma/ — „analog-rooted sound design“, glasige FM-Töne, Verzerrung; später Stilwechsel zu overdriven Synth-Pop.
- https://www.loudersound.com/features/the-bluffer-s-guide-industrial-metal — Genrebeispiele Ministry/NIN/KMFDM und deren Merkmale (optional, ergänzend).
