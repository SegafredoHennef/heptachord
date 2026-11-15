# Vier Stimmen auf vier Kanälen, plus Single-Channel-Modus

Der Output-Modus ist ein Parameter: `Multi` (Default) gibt Thru, H1, H2, H3 auf
den MIDI-Kanälen 1–4 aus; `Single` legt alle vier Stimmen auf einen Kanal, sodass
ein Empfänger einen Akkord spielt. Kanäle sind frei wählbar, müssen im Modus
`Multi` aber paarweise verschieden sein.

Grund: Hosts behandeln Mehrkanal-MIDI-Out sehr unterschiedlich. Reaper erhält
alle Kanäle, Ableton Live legt sie beim internen Routing zu einem Kanal zusammen,
Studio One hört pro Track genau einen Kanal, Cubase und Bitwig sind nicht
dokumentiert. Ohne `Single` wäre Heptachord in Live und Studio One nur eingeschränkt
brauchbar.

## Consequences

- Der Core muss die Kanalzuordnung als Daten führen, nicht als Konstante; die UI
  verhindert Doppelbelegung im Modus `Multi`.
- Notenhaushalt: Pitchwechsel innerhalb eines Steps beendet die alte Note und
  startet die neue am Step-Rand (kein Pitchbend); bei Transport-Stop,
  Preset-Load und „alle Noten aus" geht pro belegtem Kanal ein All-Notes-Off
  raus.
