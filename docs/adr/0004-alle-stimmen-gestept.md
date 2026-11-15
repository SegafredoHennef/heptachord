# Alle vier Stimmen laufen im Step-Raster

Die Thru-Stimme ist keine echte Durchreiche: Alle vier Stimmen — Thru, H1, H2,
H3 — werden an den Step-Grenzen getriggert. Thru unterscheidet sich nur durch
die Tonhöhe (unveränderte Eingangsnote statt diatonischem Gradabstand), nicht
durch die Zeitachse.

Damit sind Hold, Rest und Ratchet für jede Stimme gleich definiert, die
Step-Karten zeigen pro Step vier An/Aus-Punkte, und die 12×8×4-Abnahmetabelle
hat für alle Stimmen dieselbe Struktur. Die naheliegende Alternative — Thru
klingt sofort und folgt dem Tastendruck, nur H1–H3 sind gestept — hätte zwei
verschiedene Zeitmodelle in einem Gerät erzeugt.

## Consequences

- Das Spielen fühlt sich durchgehend gestept an, auch für die Originalnote. Wer
  die trockene Note ungestept will, routet sie in der DAW direkt am Plugin
  vorbei.
- Note-On ohne Step-Grenze erzeugt keinen sofortigen Klang; das erste Gate
  kommt am nächsten Raster.
