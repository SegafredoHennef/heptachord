# Eine Parameter-Tabelle als Single Source of Truth

Alle Parameter (ID, Typ, Bereich, Default, UI-Metadaten) stehen in
`parameters.toml`, aus der C++-Header, UI-Bindings und das Preset-Schema
generiert werden. Die Markdown-Tabelle in `docs/02-parameter.md` ist daraus
generierte Doku, nicht die Quelle. Presets sind JSON-Dateien (`.heptachord`) mit
`schemaVersion`; der VST3-State ist dasselbe JSON. Parameter-IDs werden in M0
eingefroren.

Ohne diese Regel müssten APVTS-Baum, Editor-Layout und Preset-Serialisierung
dreifach gepflegt werden — genau die Stellen, an denen später
Preset-Roundtrips (Abnahmekriterium 4) und Schema-Migrationen brechen. Die
Alternative (Parameter von Hand in APVTS plus separate UI-Konstanten) wäre
schneller zu beginnen, aber nach dem ersten Release nicht mehr sauber zu
ändern.

## Consequences

- Die Tabelle ist ein Build-Schritt; ein generierter Header darf nicht von Hand
  editiert werden.
- Jede Parameter-Änderung ist eine Schema-Änderung und braucht eine
  Migrationsregel im Preset-Loader.
- `tests/parameter_ids.txt` (Golden File) enthält die ID-Liste in fester
  Reihenfolge; CI schlägt fehl, sobald sich eine ID ändert.
