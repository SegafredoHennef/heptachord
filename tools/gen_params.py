#!/usr/bin/env python3
"""Generiert aus parameters.toml die C++-Tabelle, die Doku und das Golden File.

Aufruf:
    python tools/gen_params.py            # schreiben
    python tools/gen_params.py --check    # nur pruefen (CI), Exit 1 bei Drift

Nur Standardbibliothek (tomllib ab Python 3.11). Keine Fremdabhaengigkeiten.
"""

from __future__ import annotations

import sys
import tomllib
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "parameters.toml"
HEADER = ROOT / "src" / "core" / "ParameterTable.h"
DOC = ROOT / "docs" / "02-parameter.md"
IDS = ROOT / "tests" / "parameter_ids.txt"

VALID_TYPES = {"bool", "int", "float", "choice", "trigger"}


class Param:
    def __init__(self, pid, group, label, ptype, pmin=None, pmax=None,
                 default=None, unit=None, choices=None, note=None):
        self.id = pid
        self.group = group
        self.label = label
        self.type = ptype
        self.min = pmin
        self.max = pmax
        self.default = default
        self.unit = unit
        self.choices = choices
        self.note = note

    @property
    def default_index(self):
        if self.type == "choice":
            return self.choices.index(self.default)
        return None

    @property
    def default_value(self):
        if self.type == "choice":
            return float(self.default_index)
        if self.type == "bool":
            return 1.0 if self.default else 0.0
        if self.type == "trigger":
            return 0.0
        return float(self.default)


def load() -> tuple[int, list[Param], list[tuple[str, str]]]:
    with SOURCE.open("rb") as fh:
        data = tomllib.load(fh)

    schema_version = int(data.get("schema_version", 0))
    if schema_version < 1:
        raise SystemExit("parameters.toml: schema_version fehlt oder ist < 1")

    groups = [(g["id"], g["label"]) for g in data.get("group", [])]
    known_groups = {gid for gid, _ in groups}

    params: list[Param] = []
    for p in data.get("param", []):
        params.append(_make(p, p["id"], p["group"], p["label"]))

    for fam in data.get("family", []):
        count = int(fam["count"])
        if count < 1:
            raise SystemExit(f"family {fam['id']}: count muss >= 1 sein")
        for i in range(1, count + 1):
            for spec in fam.get("param", []):
                label = spec["label"].replace("{n}", str(i))
                if fam["label"].startswith("S{") or fam["label"].startswith("H{"):
                    label = fam["label"].replace("{n}", str(i)) + " · " + label
                params.append(_make(spec, f"{fam['id']}{i}.{spec['name']}",
                                    fam["group"], label, i, count))

    for p in params:
        if p.group not in known_groups:
            raise SystemExit(f"{p.id}: unbekannte Gruppe {p.group!r}")
    _validate(params)
    return schema_version, params, groups


def _make(spec, pid, group, label, index=None, count=None) -> Param:
    ptype = spec["type"]
    if ptype not in VALID_TYPES:
        raise SystemExit(f"{pid}: unbekannter Typ {ptype!r}")
    default = spec.get("default")
    if isinstance(default, list):
        if index is None or count is None:
            raise SystemExit(f"{pid}: Default-Liste nur in Familien erlaubt")
        if len(default) != count:
            raise SystemExit(f"{pid}: Default-Liste hat {len(default)} statt {count} Werte")
        default = default[index - 1]
    return Param(
        pid=pid,
        group=group,
        label=label,
        ptype=ptype,
        pmin=spec.get("min"),
        pmax=spec.get("max"),
        default=default,
        unit=spec.get("unit"),
        choices=list(spec["choices"]) if "choices" in spec else None,
        note=spec.get("note"),
    )


def _validate(params: list[Param]) -> None:
    seen: set[str] = set()
    for p in params:
        if p.id in seen:
            raise SystemExit(f"doppelte ID: {p.id}")
        seen.add(p.id)

        if not all(c.isalnum() or c in "._" for c in p.id):
            raise SystemExit(f"{p.id}: unerlaubtes Zeichen in der ID")
        if p.id != p.id.lower():
            raise SystemExit(f"{p.id}: IDs sind kleingeschrieben")

        if p.type == "choice":
            if not p.choices or len(p.choices) < 2:
                raise SystemExit(f"{p.id}: choice braucht mindestens zwei Werte")
            if p.default not in p.choices:
                raise SystemExit(f"{p.id}: Default {p.default!r} nicht in den Auswahlwerten")
        elif p.type in {"int", "float"}:
            if p.min is None or p.max is None:
                raise SystemExit(f"{p.id}: min/max fehlen")
            if p.min >= p.max:
                raise SystemExit(f"{p.id}: min muss kleiner als max sein")
            if not (p.min <= p.default <= p.max):
                raise SystemExit(f"{p.id}: Default {p.default} liegt ausserhalb {p.min}..{p.max}")
            if p.type == "int" and not float(p.default).is_integer():
                raise SystemExit(f"{p.id}: int-Default ist nicht ganzzahlig")
        elif p.type == "bool":
            if not isinstance(p.default, bool):
                raise SystemExit(f"{p.id}: bool braucht true/false")
        elif p.type == "trigger":
            if p.default is not None:
                raise SystemExit(f"{p.id}: trigger hat keinen Default")


def _f(value) -> str:
    # C++ verlangt bei Float-Literalen einen Dezimalpunkt oder Exponenten:
    # "2f" ist ungueltig, "2.0f" nicht. %g laesst den Punkt bei Ganzzahlen weg.
    text = f"{float(value):g}"
    if "." not in text and "e" not in text and "E" not in text:
        text += ".0"
    return f"{text}f"


def render_header(schema_version: int, params: list[Param]) -> str:
    out = [
        "// Generiert aus parameters.toml - nicht von Hand bearbeiten.",
        "// Neu erzeugen: python tools/gen_params.py",
        "#pragma once",
        "",
        "#include <array>",
        "#include <cstdint>",
        "#include <cstring>",
        "",
        "namespace heptachord::params {",
        "",
        "enum class Type : std::uint8_t { Bool, Int, Float, Choice, Trigger };",
        "",
        "struct Param {",
        "    const char* id;",
        "    const char* label;",
        "    const char* group;",
        "    Type type;",
        "    float min;",
        "    float max;",
        "    float defaultValue;",
        "    const char* unit;",
        "    const char* const* choices;",
        "    int numChoices;",
        "    const char* note;",
        "};",
        "",
        f"inline constexpr int kSchemaVersion = {schema_version};",
        f"inline constexpr int kNumParameters = {len(params)};",
        "",
    ]

    for i, p in enumerate(params):
        if p.choices:
            values = ", ".join(f'"{c}"' for c in p.choices)
            out.append(f"inline constexpr const char* kChoices{i}[] = {{ {values} }};")

    def lit(value):
        return "nullptr" if value is None else f'"{value}"'

    out.append("")
    out.append("inline constexpr std::array<Param, static_cast<std::size_t>(kNumParameters)> kParams = {{")
    for i, p in enumerate(params):
        pmin = _f(p.min) if p.min is not None else "0.0f"
        pmax = _f(p.max) if p.max is not None else "0.0f"
        choices = f"kChoices{i}" if p.choices else "nullptr"
        out.append(
            f'    {{ "{p.id}", "{p.label}", "{p.group}", Type::{p.type.capitalize()}, '
            f"{pmin}, {pmax}, {_f(p.default_value)}, {lit(p.unit)}, {choices}, "
            f"{len(p.choices) if p.choices else 0}, {lit(p.note)} }},"
        )
    out.append("}};")
    out.append("")

    out.append("// Index-Konstanten (Reihenfolge = Golden File)")
    for i, p in enumerate(params):
        name = p.id.replace(".", "_")
        out.append(f"inline constexpr int id_{name} = {i};")
    out.append("")
    out.append('inline int indexOf (const char* id) noexcept')
    out.append("{")
    out.append("    for (int i = 0; i < kNumParameters; ++i)")
    out.append("        if (std::strcmp (kParams[static_cast<std::size_t> (i)].id, id) == 0)")
    out.append("            return i;")
    out.append("    return -1;")
    out.append("}")
    out.append("")
    out.append("} // namespace heptachord::params")
    out.append("")
    return "\n".join(out)


def _range_text(p: Param) -> str:
    if p.type == "choice":
        return f"0…{len(p.choices) - 1} ({' / '.join(p.choices)})"
    if p.type == "bool":
        return "an/aus"
    if p.type == "trigger":
        return "—"
    unit = f" {p.unit}" if p.unit else ""
    return f"{p.min:g}…{p.max:g}{unit}"


def _default_text(p: Param) -> str:
    if p.type == "bool":
        return "an" if p.default else "aus"
    if p.type == "trigger":
        return "—"
    if p.type == "choice":
        return str(p.default)
    unit = f" {p.unit}" if p.unit else ""
    return f"{p.default:g}{unit}"


def render_doc(schema_version: int, params: list[Param], groups) -> str:
    out = [
        "# 02 — Parameter (v1)",
        "",
        f"Generiert aus `parameters.toml` (schemaVersion {schema_version}), nicht von Hand",
        "bearbeiten. Die Tabelle ist die Quelle für `src/core/ParameterTable.h`, die",
        "UI-Metadaten und das Preset-Schema (ADR 0003).",
        "",
        "**ID-Schema**: `<gruppe>.<name>`, ASCII, kleingeschrieben. Globale Parameter",
        "haben kein Präfix. Einzige erlaubte Indexform ist `s1…s8` und `h1…h3`.",
        "IDs sind eingefroren; `tests/parameter_ids.txt` ist das Golden File.",
        "",
        "**Preset-Kopf** (kein Parameter): `schemaVersion`, `name`, `author`, `notes`.",
        "",
        "**Einheiten**: Velocity immer in Prozent (0…200 %, Default 100 %). Intern",
        "multiplikativ, geklemmt auf 1…127.",
        "",
        f"**Summe**: {len(params)} Parameter.",
        "",
    ]

    for gid, glabel in groups:
        rows = [p for p in params if p.group == gid]
        if not rows:
            continue
        out.append(f"## {glabel}")
        out.append("")
        out.append("| ID | Label | Typ | Bereich | Default | Anmerkung |")
        out.append("|---|---|---|---|---|---|")
        for p in rows:
            note = (p.note or "").replace("|", "\\|")
            out.append(
                f"| `{p.id}` | {p.label} | {p.type} | {_range_text(p)} | "
                f"{_default_text(p)} | {note} |"
            )
        out.append("")

    out.extend([
        "## Kanalregeln",
        "",
        "- Modus `Multi`: `thru.channel` und `h1…h3.channel` paarweise verschieden,",
        "  die UI verhindert Doppelbelegung.",
        "- Modus `Single`: `thru.channel` ist der Master-Kanal, die `hN.channel`-Werte",
        "  werden ignoriert, bleiben aber gespeichert.",
        "- Umschalten des Modus beendet am nächsten Step-Rand alle Noten auf den alten",
        "  Kanälen (Note-Off) und triggert auf den neuen.",
        "",
    ])
    return "\n".join(out)


def main(argv: list[str]) -> int:
    check = "--check" in argv
    schema_version, params, groups = load()

    outputs = {
        HEADER: render_header(schema_version, params),
        DOC: render_doc(schema_version, params, groups),
        IDS: "\n".join(p.id for p in params) + "\n",
    }

    drift = []
    for path, text in outputs.items():
        if check:
            current = path.read_text(encoding="utf-8") if path.exists() else ""
            if current.replace("\r\n", "\n") != text:
                drift.append(path)
        else:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(text, encoding="utf-8", newline="\n")

    if check:
        if drift:
            for path in drift:
                print(f"Drift: {path.relative_to(ROOT)}")
            print("Bitte 'python tools/gen_params.py' ausfuehren.")
            return 1
        print(f"Parameter-Tabelle aktuell ({len(params)} Parameter).")
        return 0

    print(f"{len(params)} Parameter geschrieben:")
    for path in outputs:
        print(f"  {path.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
