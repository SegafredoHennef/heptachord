# Produktidentität: Heptachord / Blakkmoon Audio

Status: accepted (Name geprüft am 2026-11-15, formale Registerprüfung offen)

Das Plugin heißt **Heptachord** — der Sieben-Ton-Vorrat, also genau das
Skalenmodell, auf dem das Gerät arbeitet. Hersteller ist **Blakkmoon Audio**,
Bundle-ID `com.blakkmoon.heptachord`, Preset-Endung `.heptachord`.

Der Arbeitstitel *Asterism* war im Audio-Umfeld belegt (Moya Synths „Asterism",
ein scale-aware Instrument; ein KVR-Developer-Challenge-Eintrag 2026; ein
Modular-Synth). Ebenso verworfen: *Asterion* (AtomoSynth-Hardware), *Sidereal*
(SYN-8800 von OM IS), *Tessera* (Audio Damage).

Vorprüfung: keine Produkttreffer für „Heptachord" und keine Treffer für
„Blakkmoon"; einziger Audio-Treffer ist ein Track-Titel „Heptachord" (Avidus,
Empore Music auf Beatport) — kein Produktname in der Plugin-Klasse.
`heptachord.com` ist registriert (Shopify-Platzhalter „Opening soon"),
`heptachord.de/.net/.audio` und `blakkmoon.de/.net/.audio` antworten nicht im
DNS. `blakkmoon.com` ist registriert.

## Consequences

- JUCE-Konstanten: `COMPANY_NAME "Blakkmoon Audio"`,
  `BUNDLE_ID "com.blakkmoon.heptachord"`, `PLUGIN_CODE "Hptc"`,
  `PLUGIN_MANUFACTURER_CODE "Blkk"`.
- Der Name steckt in Repo-Ordner, Artefaktnamen, Preset-Endung und
  Release-Texten; eine spätere Umbenennung ist eine Migration inklusive
  Preset-Kompatibilität.
- **Offen (vor dem ersten Release):** DPMA-, EUIPO- und USPTO-Registerrecherche
  in Klasse 9/41 sowie endgültige Domainwahl. Ein Domainkauf ist keine
  Voraussetzung für M0.
