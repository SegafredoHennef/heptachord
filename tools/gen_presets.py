#!/usr/bin/env python3
"""Erzeugt die 96 Factory-Presets fuer Heptachord (12 Grundtoene x 8 Skalen).

Jede Skala hat ein musikalisch interessantes Step-Muster ("Charakter",
Decision 82); die 96 Presets setzen dasselbe Muster an allen 12
chromatischen Grundtoenen um - nur `root` aendert sich. Die Anzeigenamen
(German-Kanon-Form, Grundton zuerst) leiten sich im Editor aus root + Skala
+ Bezeichnungsmodus ab; der JSON-`name` bleibt die deutsche Kanon-Form.

Achtung: Overrides muessen die exakten Parameter-IDs (mit Punkt) verwenden,
z. B. "s3.mode" - unbekannte Schluessel wie "s3_mode" wuerden beim Laden
still ignoriert (Preset::fromJson).
"""
import json
import os

# Parameter-IDs aus ParameterTable.h (Reihenfolge = Golden File).
# names.mode wird bewusst NICHT in Presets geschrieben (Decision 83).
PARAM_IDS = [
    "root", "scale", "rate", "direction", "swing", "slack", "latch",
    "output.mode", "reset",
    "thru.enable", "thru.snap", "thru.velocity", "thru.channel",
    "harm.velocity", "harm.spread",
    "h1.enable", "h1.degree", "h1.octave", "h1.velocity", "h1.follow", "h1.gate", "h1.channel",
    "h2.enable", "h2.degree", "h2.octave", "h2.velocity", "h2.follow", "h2.gate", "h2.channel",
    "h3.enable", "h3.degree", "h3.octave", "h3.velocity", "h3.follow", "h3.gate", "h3.channel",
]
for i in range(1, 9):
    PARAM_IDS.extend([
        f"s{i}.active", f"s{i}.mode", f"s{i}.offset", f"s{i}.velocity",
        f"s{i}.probability", f"s{i}.ratchet",
        f"s{i}.v1", f"s{i}.v2", f"s{i}.v3", f"s{i}.v4",
    ])

# Deutsche Kanon-Form (Grundton zuerst), Anzeige folgt names.mode.
ROOT_NAMES = ["C", "Cis", "D", "Dis", "E", "F", "Fis", "G", "Gis", "A", "Ais", "H"]
ROOT_SLUGS = ["c", "cis", "d", "dis", "e", "f", "fis", "g", "gis", "a", "ais", "h"]
SCALE_NAMES = ["Dur", "Moll", "Harmonisch Moll", "Melodisch Moll",
               "Dorisch", "Phrygisch", "Lydisch", "Mixolydisch"]
SCALE_SLUGS = ["dur", "moll", "harmonisch_moll", "melodisch_moll",
               "dorisch", "phrygisch", "lydisch", "mixolydisch"]

# Charakter-Muster pro Skala - exakt die Overrides der bisherigen Presets
# p1..p8 (v0.1.0/v0.1.1), jetzt mit den echten Parameter-IDs (Punkt statt
# Unterstrich). Reihenfolge beibehalten.
SCALE_PATTERNS = [
    # Dur: klassisch, vorwaerts, alle Steps aktiv
    {},
    # Moll: Swing 50 %, Step 3+4 als Rest/Hold fuer Atmung, Akzente
    {"swing": 50.0, "s3.mode": 2, "s4.mode": 1, "s5.velocity": 120.0, "s7.velocity": 80.0},
    # Harmonisch Moll: PingPong, 1/4, Spread weiter, versetzte Offsets
    {"rate": 4, "direction": 2, "harm.spread": 0.5, "s2.offset": 1, "s6.offset": -1},
    # Melodisch Moll: Slack, Ratchet auf Step 8, Hold auf Step 5, laengeres Gate
    {"slack": 15.0, "s8.mode": 3, "s8.ratchet": 4, "s5.mode": 1, "h3.gate": 70.0},
    # Dorisch: modal, 1/6, Wahrscheinlichkeiten, versetzte Offsets
    {"rate": 3, "s3.probability": 50.0, "s7.probability": 75.0, "s2.offset": 2, "s6.offset": -2},
    # Phrygisch: schnell (1/16), Ratchet auf Step 1, Pause, enge grade
    {"rate": 0, "s1.mode": 3, "s1.ratchet": 3, "s4.mode": 2,
     "h1.degree": 1, "h2.degree": 3, "h3.degree": 5},
    # Lydisch: vertraeumt, 1/4, Hold-Pads, lange Gates, enger Spread
    {"rate": 4, "s2.mode": 1, "s5.mode": 1, "s7.mode": 1,
     "h1.gate": 80.0, "h2.gate": 80.0, "h3.gate": 80.0, "harm.spread": 0.2},
    # Mixolydisch: Blues/Rock, rueckwaerts, Akzente
    {"direction": 1, "s1.velocity": 130.0, "s4.velocity": 120.0,
     "s5.velocity": 120.0, "s8.velocity": 130.0,
     "h1.degree": 2, "h2.degree": 3, "h3.degree": 5},
]

# Default-Werte aus parameters.toml (Choice-Parameter als Index!)
DEFAULTS = {
    "root": 0, "scale": 0, "rate": 2, "direction": 0,
    "swing": 0.0, "slack": 0.0, "latch": False, "output.mode": 0, "reset": 0,
    "thru.enable": True, "thru.snap": False, "thru.velocity": 100.0, "thru.channel": 1,
    "harm.velocity": 100.0, "harm.spread": 0.3,
    "h1.enable": True, "h1.degree": 2, "h1.octave": 0, "h1.velocity": 100.0,
    "h1.follow": True, "h1.gate": 50.0, "h1.channel": 2,
    "h2.enable": True, "h2.degree": 4, "h2.octave": 0, "h2.velocity": 100.0,
    "h2.follow": True, "h2.gate": 50.0, "h2.channel": 3,
    "h3.enable": True, "h3.degree": 6, "h3.octave": 0, "h3.velocity": 100.0,
    "h3.follow": True, "h3.gate": 50.0, "h3.channel": 4,
}
for i in range(1, 9):
    DEFAULTS[f"s{i}.active"] = True
    DEFAULTS[f"s{i}.mode"] = 0  # Play
    DEFAULTS[f"s{i}.offset"] = 0
    DEFAULTS[f"s{i}.velocity"] = 100.0
    DEFAULTS[f"s{i}.probability"] = 100.0
    DEFAULTS[f"s{i}.ratchet"] = 2
    DEFAULTS[f"s{i}.v1"] = True
    DEFAULTS[f"s{i}.v2"] = True
    DEFAULTS[f"s{i}.v3"] = True
    DEFAULTS[f"s{i}.v4"] = True


def preset(root, scale):
    """Ein Preset: Grundton root (0..11) + Skala scale (0..7) + Charakter."""
    params = dict(DEFAULTS)
    params["root"] = root
    params["scale"] = scale
    for key, value in SCALE_PATTERNS[scale].items():
        params[key] = value
    name = f"{ROOT_NAMES[root]} {SCALE_NAMES[scale]}"
    return {"schemaVersion": 1, "name": name, "parameters": params}


out_dir = os.path.join(os.path.dirname(__file__), "..", "presets")
os.makedirs(out_dir, exist_ok=True)

count = 0
for root in range(12):
    for scale in range(8):
        slot = root * 8 + scale + 1  # stabile chromatische ID (Decision 82)
        p = preset(root, scale)
        filename = f"heptachord_{slot:02d}_{ROOT_SLUGS[root]}_{SCALE_SLUGS[scale]}.heptachord"
        path = os.path.join(out_dir, filename)
        with open(path, "w", encoding="utf-8") as f:
            json.dump(p, f, indent=2, ensure_ascii=False)
        print(f"  {filename}")
        count += 1

print(f"\n{count} Factory-Presets erzeugt.")
