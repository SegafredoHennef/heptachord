# Heptachord Standalone → Reason: MIDI-Routing via loopMIDI

Heptachord gibt **kein Audio und kein CV** aus — nur MIDI-Noten (4 Stimmen auf Kanälen 1–4).
Reason hat keine interne MIDI-Routing-Matrix. Deshalb wird ein virtueller MIDI-Port (loopMIDI)
als Brücke genutzt.

## Voraussetzungen

- Heptachord Standalone (`Heptachord.exe`)
- [loopMIDI](https://www.tobias-erichsen.de/software/loopmidi.html) (kostenlos, Windows)
- Reason (beliebige Version mit VST3/RE-Support)

## Schritt 1: loopMIDI installieren und Port erstellen

1. loopMIDI herunterladen und installieren: https://www.tobias-erichsen.de/software/loopmidi.html
2. loopMIDI starten.
3. Unter **"New port name"** einen Namen eingeben, z. B. `Heptachord`.
4. **"+"** klicken → der Port erscheint in der Liste.
5. loopMIDI **laufend lassen** (Minimieren, nicht schließen).

## Schritt 2: Heptachord Standalone starten und MIDI-Out setzen

1. `Heptachord.exe` starten.
2. In Heptachord: **MIDI-Out** auf den loopMIDI-Port setzen (Dropdown "Ausgabe" oder MIDI-Out-Auswahl).
   - Falls kein Dropdown sichtbar: Heptachord nutzt den ersten verfügbaren MIDI-Out. Sicherstellen, dass loopMIDI der einzige aktive MIDI-Out ist, oder in den Einstellungen den Port wählen.
3. Eine Note spielen (Tastatur A–K) oder Latch aktivieren → Heptachord sendet MIDI-Noten auf Kanälen 1–4.

## Schritt 3: Reason — Advanced MIDI Device erstellen

1. Reason öffnen.
2. Im Rack: **Create** → **Advanced MIDI Device** (oder im Menü: Create → Advanced MIDI).
   - Das Gerät erscheint im Rack.
3. Im Advanced MIDI Device:
   - **MIDI Input:** den loopMIDI-Port wählen (`Heptachord`).
   - **MIDI Output:** das Ziel-Instrument wählen (z. B. Subtractor, Thor, Europa) oder **"Send to Track"**.
4. Das Ziel-Instrument (z. B. Subtractor) in einen Track laden.

## Schritt 4: Rack-Verkabelung (MIDI-Kabel)

1. Advanced MIDI Device → **MIDI Out** (gelbes Kabel) → Ziel-Instrument **MIDI In**.
   - Hinweis: Das ist ein MIDI-Kabel, kein Audio- oder CV-Kabel.
2. Ziel-Instrument → **Audio Out** → Mixer (Audio-Kabel).

## Schritt 5: Testen

1. Heptachord: Latch aktivieren, eine Note spielen (z. B. Taste `A`).
2. Reason-Transport starten (Space) → Heptachord folgt dem Clock.
3. Die Steps leuchten in Heptachord, und das Ziel-Instrument spielt die harmonisierten Noten.

## Troubleshooting

| Problem | Lösung |
|---|---|
| Kein Sound | Ziel-Instrument an Advanced MIDI Device angeschlossen? Audio-Kabel zum Mixer? |
| Steps leuchten, aber still | MIDI-Kabel (gelb) zwischen Advanced MIDI Device und Instrument prüfen. |
| Heptachord crasht | v0.1.4-basierten Build verwenden (siehe BACKLOG.md). |
| Falsche Noten | Skala/Grundton in Heptachord prüfen; Bezeichnungsmodus (Deutsch/International). |
| Latenz | Buffer-Size in Heptachord (Audio-Settings) reduzieren; in Reason Audio-Settings prüfen. |

## Alternative: Heptachord Standalone mit internem Synth

Heptachord Standalone hat einen eingebauten Test-Synth. Einfach:
1. Heptachord Standalone starten.
2. Eine Note spielen (Tastatur A–K).
3. Latch aktivieren.
4. Du hörst sofort Sound — ohne Routing, ohne Reason.

## Alternative: Andere DAWs

Live, Cubase, Reaper und Bitwig unterstützen VST3-MIDI-Routing nativ. Dort kann Heptachord als VST3 geladen werden, und die MIDI-Ausgabe direkt an ein anderes Instrument geroutet werden — ohne virtuelles MIDI-Kabel.

## Dateien

- Heptachord Standalone: `C:\opencode-projekte\heptachord-v014\build\ninja\heptachord_artefacts\Release\Standalone\Heptachord.exe`
- loopMIDI: https://www.tobias-erichsen.de/software/loopmidi.html