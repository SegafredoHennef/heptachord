#!/usr/bin/env python3
"""Erzeugt die 8 Factory-Presets für Heptachord.

Jedes Preset verwendet eine andere Skala und ein musikalisch
interessantes Step-Muster, damit die Klänge vielfaeltig sind.
"""
import json
import os

# Parameter-IDs aus ParameterTable.h (Reihenfolge = Golden File)
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

def preset(name, scale, **overrides):
    params = dict(DEFAULTS)
    params["scale"] = scale
    for k, v in overrides.items():
        params[k] = v
    return {"schemaVersion": 1, "name": name, "parameters": params}

# 1. C Dur — klassisch, vorwärts, alle Steps aktiv
p1 = preset("C Dur", 0)

# 2. C Moll — Moll-Tonart, Swing 50 %, Step 3+4 als Hold/Rest für Atmung
p2 = preset("C Moll", 1,
    swing=50.0,
    s3_mode=2,  # Rest
    s4_mode=1,  # Hold
    s5_velocity=120.0,  # Akzent
    s7_velocity=80.0,   # leiser
)

# 3. C Harmonisch Moll — PingPong, langsamerer Rate, Spread weiter
p3 = preset("C Harmonisch Moll", 2,
    rate=4,  # 1/4
    direction=2,  # PingPong
    harm_spread=0.5,
    s2_offset=1,  # erhöhter zweiter Step
    s6_offset=-1,
)

# 4. C Melodisch Moll — Slack für entspannten Feel, Ratchet auf Step 8
p4 = preset("C Melodisch Moll", 3,
    slack=15.0,
    s8_mode=3,  # Ratchet
    s8_ratchet=4,
    s5_mode=1,  # Hold
    h3_gate=70.0,
)

# 5. C Dorisch — modal, 1/6 Rate, Wahrscheinlichkeit auf manchen Steps
p5 = preset("C Dorisch", 4,
    rate=3,  # 1/6
    s3_probability=50.0,
    s7_probability=75.0,
    s2_offset=2,
    s6_offset=-2,
)

# 6. C Phrygisch — schnell (1/16), spanisch, Ratchet auf Step 1
p6 = preset("C Phrygisch", 5,
    rate=0,  # 1/16
    s1_mode=3,  # Ratchet
    s1_ratchet=3,
    s4_mode=2,  # Rest
    h1_degree=1,  # Halbton-Schritt für phrygischen Charakter
    h2_degree=3,
    h3_degree=5,
)

# 7. C Lydisch — verträumt, 1/4 Rate, Hold-Modus für Pads
p7 = preset("C Lydisch", 6,
    rate=4,  # 1/4
    s2_mode=1,  # Hold
    s5_mode=1,  # Hold
    s7_mode=1,  # Hold
    h1_gate=80.0,
    h2_gate=80.0,
    h3_gate=80.0,
    harm_spread=0.2,
)

# 8. C Mixolydisch — Blues/Rock, rückwärts, Akzente
p8 = preset("C Mixolydisch", 7,
    direction=1,  # Rückwärts
    s1_velocity=130.0,
    s4_velocity=120.0,
    s5_velocity=120.0,
    s8_velocity=130.0,
    h1_degree=2,
    h2_degree=3,  # kleine Terz für Mixolydisch-Charakter
    h3_degree=5,
)

presets = [p1, p2, p3, p4, p5, p6, p7, p8]

out_dir = os.path.join(os.path.dirname(__file__), "..", "presets")
os.makedirs(out_dir, exist_ok=True)

for i, p in enumerate(presets, 1):
    filename = f"heptachord_{i:02d}_{p['name'].lower().replace(' ', '_')}.heptachord"
    path = os.path.join(out_dir, filename)
    with open(path, "w", encoding="utf-8") as f:
        json.dump(p, f, indent=2, ensure_ascii=False)
    print(f"  {filename}")

print(f"\n{len(presets)} Factory-Presets erzeugt.")
