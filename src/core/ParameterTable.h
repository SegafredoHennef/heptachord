// Generiert aus parameters.toml - nicht von Hand bearbeiten.
// Neu erzeugen: python tools/gen_params.py
#pragma once

#include <array>
#include <cstdint>
#include <cstring>

namespace heptachord::params {

enum class Type : std::uint8_t { Bool, Int, Float, Choice, Trigger };

struct Param {
    const char* id;
    const char* label;
    const char* group;
    Type type;
    float min;
    float max;
    float defaultValue;
    const char* unit;
    const char* const* choices;
    int numChoices;
    const char* note;
};

inline constexpr int kSchemaVersion = 1;
inline constexpr int kNumParameters = 117;

inline constexpr const char* kChoices0[] = { "C", "Cis", "D", "Dis", "E", "F", "Fis", "G", "Gis", "A", "Ais", "H" };
inline constexpr const char* kChoices1[] = { "Dur", "Moll", "Harmonisch Moll", "Melodisch Moll", "Dorisch", "Phrygisch", "Lydisch", "Mixolydisch" };
inline constexpr const char* kChoices2[] = { "1/16", "1/12", "1/8", "1/6", "1/4", "1/3", "3/8", "1/2", "2/3", "3/4", "1", "4/3", "3/2", "2", "3", "4", "8" };
inline constexpr const char* kChoices3[] = { "Vorwärts", "Rückwärts", "Pendel I", "Pendel II", "Brownian", "Zufall" };
inline constexpr const char* kChoices7[] = { "Multi", "Single" };
inline constexpr const char* kChoices9[] = { "Deutsch", "International (b)", "International (#)" };
inline constexpr const char* kChoices38[] = { "Play", "Hold", "Rest", "Ratchet" };
inline constexpr const char* kChoices48[] = { "Play", "Hold", "Rest", "Ratchet" };
inline constexpr const char* kChoices58[] = { "Play", "Hold", "Rest", "Ratchet" };
inline constexpr const char* kChoices68[] = { "Play", "Hold", "Rest", "Ratchet" };
inline constexpr const char* kChoices78[] = { "Play", "Hold", "Rest", "Ratchet" };
inline constexpr const char* kChoices88[] = { "Play", "Hold", "Rest", "Ratchet" };
inline constexpr const char* kChoices98[] = { "Play", "Hold", "Rest", "Ratchet" };
inline constexpr const char* kChoices108[] = { "Play", "Hold", "Rest", "Ratchet" };

inline constexpr std::array<Param, static_cast<std::size_t>(kNumParameters)> kParams = {{
    { "root", "Grundton", "global", Type::Choice, 0.0f, 0.0f, 0.0f, nullptr, kChoices0, 12, "Bezugston der Skala; Anzeige folgt names.mode" },
    { "scale", "Skala", "global", Type::Choice, 0.0f, 0.0f, 0.0f, nullptr, kChoices1, 8, "Reihenfolge wie im Original" },
    { "rate", "Rate", "global", Type::Choice, 0.0f, 0.0f, 2.0f, nullptr, kChoices2, 17, "17 Werte, 1.0 = ein Takt (4/4)" },
    { "direction", "Richtung", "global", Type::Choice, 0.0f, 0.0f, 0.0f, nullptr, kChoices3, 6, "läuft nur über aktive Steps" },
    { "swing", "Swing", "global", Type::Float, 0.0f, 75.0f, 0.0f, "%", nullptr, 0, "verzögert die ungeraden Steps" },
    { "slack", "Slack", "global", Type::Float, -100.0f, 100.0f, 0.0f, "ms", nullptr, 0, "globaler Versatz, auf weniger als eine Step-Länge geklemmt" },
    { "latch", "Latch", "global", Type::Bool, 0.0f, 0.0f, 0.0f, nullptr, nullptr, 0, "hält die zuletzt gespielte Note" },
    { "output.mode", "Ausgabe", "global", Type::Choice, 0.0f, 0.0f, 0.0f, nullptr, kChoices7, 2, "Multi = eine Stimme pro Kanal, Single = alle auf thru.channel" },
    { "reset", "Reset", "global", Type::Trigger, 0.0f, 0.0f, 0.0f, nullptr, nullptr, 0, "setzt am nächsten Step-Rand auf Step 1" },
    { "names.mode", "Bezeichnungen", "global", Type::Choice, 0.0f, 0.0f, 0.0f, nullptr, kChoices9, 3, "Anzeige-Konvention fuer Noten- und Skalennamen; wird beim Laden von Presets nicht ueberschrieben" },
    { "thru.enable", "Aktiv", "thru", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, nullptr },
    { "thru.snap", "Thru mitrasten", "thru", Type::Bool, 0.0f, 0.0f, 0.0f, nullptr, nullptr, 0, "an = auch Thru wird auf die Skala gesnappt" },
    { "thru.velocity", "Velocity", "thru", Type::Float, 0.0f, 200.0f, 100.0f, "%", nullptr, 0, "Faktor auf die Eingabe-Velocity" },
    { "thru.channel", "Kanal", "thru", Type::Int, 1.0f, 16.0f, 1.0f, nullptr, nullptr, 0, "im Modus Single der Master-Kanal" },
    { "harm.velocity", "Velocity H1–H3", "harm", Type::Float, 0.0f, 200.0f, 100.0f, "%", nullptr, 0, nullptr },
    { "harm.spread", "Spread", "harm", Type::Float, 0.0f, 1.0f, 0.3f, nullptr, nullptr, 0, "spread_i = 1 − spread × (i−1)/2, i = H1…H3" },
    { "h1.enable", "H1 · Aktiv", "voice", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, nullptr },
    { "h1.degree", "H1 · Grad", "voice", Type::Int, -14.0f, 14.0f, 2.0f, nullptr, nullptr, 0, "diatonische Skalenschritte" },
    { "h1.octave", "H1 · Oktave", "voice", Type::Int, -3.0f, 3.0f, 0.0f, nullptr, nullptr, 0, "chromatisch, nach degree" },
    { "h1.velocity", "H1 · Velocity", "voice", Type::Float, 0.0f, 200.0f, 100.0f, "%", nullptr, 0, "wird von follow überstimmt" },
    { "h1.follow", "H1 · Vel. folgt", "voice", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "an = velocity wird ignoriert" },
    { "h1.gate", "H1 · Gate", "voice", Type::Float, 5.0f, 100.0f, 50.0f, "%", nullptr, 0, "relativ zur Step-Länge, an der Step-Kante geklemmt" },
    { "h1.channel", "H1 · Kanal", "voice", Type::Int, 1.0f, 16.0f, 2.0f, nullptr, nullptr, 0, "nur im Modus Multi" },
    { "h2.enable", "H2 · Aktiv", "voice", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, nullptr },
    { "h2.degree", "H2 · Grad", "voice", Type::Int, -14.0f, 14.0f, 4.0f, nullptr, nullptr, 0, "diatonische Skalenschritte" },
    { "h2.octave", "H2 · Oktave", "voice", Type::Int, -3.0f, 3.0f, 0.0f, nullptr, nullptr, 0, "chromatisch, nach degree" },
    { "h2.velocity", "H2 · Velocity", "voice", Type::Float, 0.0f, 200.0f, 100.0f, "%", nullptr, 0, "wird von follow überstimmt" },
    { "h2.follow", "H2 · Vel. folgt", "voice", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "an = velocity wird ignoriert" },
    { "h2.gate", "H2 · Gate", "voice", Type::Float, 5.0f, 100.0f, 50.0f, "%", nullptr, 0, "relativ zur Step-Länge, an der Step-Kante geklemmt" },
    { "h2.channel", "H2 · Kanal", "voice", Type::Int, 1.0f, 16.0f, 3.0f, nullptr, nullptr, 0, "nur im Modus Multi" },
    { "h3.enable", "H3 · Aktiv", "voice", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, nullptr },
    { "h3.degree", "H3 · Grad", "voice", Type::Int, -14.0f, 14.0f, 6.0f, nullptr, nullptr, 0, "diatonische Skalenschritte" },
    { "h3.octave", "H3 · Oktave", "voice", Type::Int, -3.0f, 3.0f, 0.0f, nullptr, nullptr, 0, "chromatisch, nach degree" },
    { "h3.velocity", "H3 · Velocity", "voice", Type::Float, 0.0f, 200.0f, 100.0f, "%", nullptr, 0, "wird von follow überstimmt" },
    { "h3.follow", "H3 · Vel. folgt", "voice", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "an = velocity wird ignoriert" },
    { "h3.gate", "H3 · Gate", "voice", Type::Float, 5.0f, 100.0f, 50.0f, "%", nullptr, 0, "relativ zur Step-Länge, an der Step-Kante geklemmt" },
    { "h3.channel", "H3 · Kanal", "voice", Type::Int, 1.0f, 16.0f, 4.0f, nullptr, nullptr, 0, "nur im Modus Multi" },
    { "s1.active", "S1 · Aktiv", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "aus = Slot fällt aus dem Zyklus" },
    { "s1.mode", "S1 · Modus", "step", Type::Choice, 0.0f, 0.0f, 0.0f, nullptr, kChoices38, 4, nullptr },
    { "s1.offset", "S1 · Offset", "step", Type::Int, -7.0f, 7.0f, 0.0f, nullptr, nullptr, 0, "diatonisch, nur H1–H3" },
    { "s1.velocity", "S1 · Velocity", "step", Type::Float, 0.0f, 200.0f, 100.0f, "%", nullptr, 0, "Akzente über 100 %" },
    { "s1.probability", "S1 · Wahrsch.", "step", Type::Float, 0.0f, 100.0f, 100.0f, "%", nullptr, 0, "wirkt nur bei Play und Ratchet" },
    { "s1.ratchet", "S1 · Ratchet", "step", Type::Int, 2.0f, 8.0f, 2.0f, nullptr, nullptr, 0, "nur im Modus Ratchet" },
    { "s1.v1", "S1 · Stimme 1", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: Thru" },
    { "s1.v2", "S1 · Stimme 2", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: H1" },
    { "s1.v3", "S1 · Stimme 3", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: H2" },
    { "s1.v4", "S1 · Stimme 4", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: H3" },
    { "s2.active", "S2 · Aktiv", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "aus = Slot fällt aus dem Zyklus" },
    { "s2.mode", "S2 · Modus", "step", Type::Choice, 0.0f, 0.0f, 0.0f, nullptr, kChoices48, 4, nullptr },
    { "s2.offset", "S2 · Offset", "step", Type::Int, -7.0f, 7.0f, 0.0f, nullptr, nullptr, 0, "diatonisch, nur H1–H3" },
    { "s2.velocity", "S2 · Velocity", "step", Type::Float, 0.0f, 200.0f, 100.0f, "%", nullptr, 0, "Akzente über 100 %" },
    { "s2.probability", "S2 · Wahrsch.", "step", Type::Float, 0.0f, 100.0f, 100.0f, "%", nullptr, 0, "wirkt nur bei Play und Ratchet" },
    { "s2.ratchet", "S2 · Ratchet", "step", Type::Int, 2.0f, 8.0f, 2.0f, nullptr, nullptr, 0, "nur im Modus Ratchet" },
    { "s2.v1", "S2 · Stimme 1", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: Thru" },
    { "s2.v2", "S2 · Stimme 2", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: H1" },
    { "s2.v3", "S2 · Stimme 3", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: H2" },
    { "s2.v4", "S2 · Stimme 4", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: H3" },
    { "s3.active", "S3 · Aktiv", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "aus = Slot fällt aus dem Zyklus" },
    { "s3.mode", "S3 · Modus", "step", Type::Choice, 0.0f, 0.0f, 0.0f, nullptr, kChoices58, 4, nullptr },
    { "s3.offset", "S3 · Offset", "step", Type::Int, -7.0f, 7.0f, 0.0f, nullptr, nullptr, 0, "diatonisch, nur H1–H3" },
    { "s3.velocity", "S3 · Velocity", "step", Type::Float, 0.0f, 200.0f, 100.0f, "%", nullptr, 0, "Akzente über 100 %" },
    { "s3.probability", "S3 · Wahrsch.", "step", Type::Float, 0.0f, 100.0f, 100.0f, "%", nullptr, 0, "wirkt nur bei Play und Ratchet" },
    { "s3.ratchet", "S3 · Ratchet", "step", Type::Int, 2.0f, 8.0f, 2.0f, nullptr, nullptr, 0, "nur im Modus Ratchet" },
    { "s3.v1", "S3 · Stimme 1", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: Thru" },
    { "s3.v2", "S3 · Stimme 2", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: H1" },
    { "s3.v3", "S3 · Stimme 3", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: H2" },
    { "s3.v4", "S3 · Stimme 4", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: H3" },
    { "s4.active", "S4 · Aktiv", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "aus = Slot fällt aus dem Zyklus" },
    { "s4.mode", "S4 · Modus", "step", Type::Choice, 0.0f, 0.0f, 0.0f, nullptr, kChoices68, 4, nullptr },
    { "s4.offset", "S4 · Offset", "step", Type::Int, -7.0f, 7.0f, 0.0f, nullptr, nullptr, 0, "diatonisch, nur H1–H3" },
    { "s4.velocity", "S4 · Velocity", "step", Type::Float, 0.0f, 200.0f, 100.0f, "%", nullptr, 0, "Akzente über 100 %" },
    { "s4.probability", "S4 · Wahrsch.", "step", Type::Float, 0.0f, 100.0f, 100.0f, "%", nullptr, 0, "wirkt nur bei Play und Ratchet" },
    { "s4.ratchet", "S4 · Ratchet", "step", Type::Int, 2.0f, 8.0f, 2.0f, nullptr, nullptr, 0, "nur im Modus Ratchet" },
    { "s4.v1", "S4 · Stimme 1", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: Thru" },
    { "s4.v2", "S4 · Stimme 2", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: H1" },
    { "s4.v3", "S4 · Stimme 3", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: H2" },
    { "s4.v4", "S4 · Stimme 4", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: H3" },
    { "s5.active", "S5 · Aktiv", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "aus = Slot fällt aus dem Zyklus" },
    { "s5.mode", "S5 · Modus", "step", Type::Choice, 0.0f, 0.0f, 0.0f, nullptr, kChoices78, 4, nullptr },
    { "s5.offset", "S5 · Offset", "step", Type::Int, -7.0f, 7.0f, 0.0f, nullptr, nullptr, 0, "diatonisch, nur H1–H3" },
    { "s5.velocity", "S5 · Velocity", "step", Type::Float, 0.0f, 200.0f, 100.0f, "%", nullptr, 0, "Akzente über 100 %" },
    { "s5.probability", "S5 · Wahrsch.", "step", Type::Float, 0.0f, 100.0f, 100.0f, "%", nullptr, 0, "wirkt nur bei Play und Ratchet" },
    { "s5.ratchet", "S5 · Ratchet", "step", Type::Int, 2.0f, 8.0f, 2.0f, nullptr, nullptr, 0, "nur im Modus Ratchet" },
    { "s5.v1", "S5 · Stimme 1", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: Thru" },
    { "s5.v2", "S5 · Stimme 2", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: H1" },
    { "s5.v3", "S5 · Stimme 3", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: H2" },
    { "s5.v4", "S5 · Stimme 4", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: H3" },
    { "s6.active", "S6 · Aktiv", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "aus = Slot fällt aus dem Zyklus" },
    { "s6.mode", "S6 · Modus", "step", Type::Choice, 0.0f, 0.0f, 0.0f, nullptr, kChoices88, 4, nullptr },
    { "s6.offset", "S6 · Offset", "step", Type::Int, -7.0f, 7.0f, 0.0f, nullptr, nullptr, 0, "diatonisch, nur H1–H3" },
    { "s6.velocity", "S6 · Velocity", "step", Type::Float, 0.0f, 200.0f, 100.0f, "%", nullptr, 0, "Akzente über 100 %" },
    { "s6.probability", "S6 · Wahrsch.", "step", Type::Float, 0.0f, 100.0f, 100.0f, "%", nullptr, 0, "wirkt nur bei Play und Ratchet" },
    { "s6.ratchet", "S6 · Ratchet", "step", Type::Int, 2.0f, 8.0f, 2.0f, nullptr, nullptr, 0, "nur im Modus Ratchet" },
    { "s6.v1", "S6 · Stimme 1", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: Thru" },
    { "s6.v2", "S6 · Stimme 2", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: H1" },
    { "s6.v3", "S6 · Stimme 3", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: H2" },
    { "s6.v4", "S6 · Stimme 4", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: H3" },
    { "s7.active", "S7 · Aktiv", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "aus = Slot fällt aus dem Zyklus" },
    { "s7.mode", "S7 · Modus", "step", Type::Choice, 0.0f, 0.0f, 0.0f, nullptr, kChoices98, 4, nullptr },
    { "s7.offset", "S7 · Offset", "step", Type::Int, -7.0f, 7.0f, 0.0f, nullptr, nullptr, 0, "diatonisch, nur H1–H3" },
    { "s7.velocity", "S7 · Velocity", "step", Type::Float, 0.0f, 200.0f, 100.0f, "%", nullptr, 0, "Akzente über 100 %" },
    { "s7.probability", "S7 · Wahrsch.", "step", Type::Float, 0.0f, 100.0f, 100.0f, "%", nullptr, 0, "wirkt nur bei Play und Ratchet" },
    { "s7.ratchet", "S7 · Ratchet", "step", Type::Int, 2.0f, 8.0f, 2.0f, nullptr, nullptr, 0, "nur im Modus Ratchet" },
    { "s7.v1", "S7 · Stimme 1", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: Thru" },
    { "s7.v2", "S7 · Stimme 2", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: H1" },
    { "s7.v3", "S7 · Stimme 3", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: H2" },
    { "s7.v4", "S7 · Stimme 4", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: H3" },
    { "s8.active", "S8 · Aktiv", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "aus = Slot fällt aus dem Zyklus" },
    { "s8.mode", "S8 · Modus", "step", Type::Choice, 0.0f, 0.0f, 0.0f, nullptr, kChoices108, 4, nullptr },
    { "s8.offset", "S8 · Offset", "step", Type::Int, -7.0f, 7.0f, 0.0f, nullptr, nullptr, 0, "diatonisch, nur H1–H3" },
    { "s8.velocity", "S8 · Velocity", "step", Type::Float, 0.0f, 200.0f, 100.0f, "%", nullptr, 0, "Akzente über 100 %" },
    { "s8.probability", "S8 · Wahrsch.", "step", Type::Float, 0.0f, 100.0f, 100.0f, "%", nullptr, 0, "wirkt nur bei Play und Ratchet" },
    { "s8.ratchet", "S8 · Ratchet", "step", Type::Int, 2.0f, 8.0f, 2.0f, nullptr, nullptr, 0, "nur im Modus Ratchet" },
    { "s8.v1", "S8 · Stimme 1", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: Thru" },
    { "s8.v2", "S8 · Stimme 2", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: H1" },
    { "s8.v3", "S8 · Stimme 3", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: H2" },
    { "s8.v4", "S8 · Stimme 4", "step", Type::Bool, 0.0f, 0.0f, 1.0f, nullptr, nullptr, 0, "Stimmenmaske: H3" },
}};

// Index-Konstanten (Reihenfolge = Golden File)
inline constexpr int id_root = 0;
inline constexpr int id_scale = 1;
inline constexpr int id_rate = 2;
inline constexpr int id_direction = 3;
inline constexpr int id_swing = 4;
inline constexpr int id_slack = 5;
inline constexpr int id_latch = 6;
inline constexpr int id_output_mode = 7;
inline constexpr int id_reset = 8;
inline constexpr int id_names_mode = 9;
inline constexpr int id_thru_enable = 10;
inline constexpr int id_thru_snap = 11;
inline constexpr int id_thru_velocity = 12;
inline constexpr int id_thru_channel = 13;
inline constexpr int id_harm_velocity = 14;
inline constexpr int id_harm_spread = 15;
inline constexpr int id_h1_enable = 16;
inline constexpr int id_h1_degree = 17;
inline constexpr int id_h1_octave = 18;
inline constexpr int id_h1_velocity = 19;
inline constexpr int id_h1_follow = 20;
inline constexpr int id_h1_gate = 21;
inline constexpr int id_h1_channel = 22;
inline constexpr int id_h2_enable = 23;
inline constexpr int id_h2_degree = 24;
inline constexpr int id_h2_octave = 25;
inline constexpr int id_h2_velocity = 26;
inline constexpr int id_h2_follow = 27;
inline constexpr int id_h2_gate = 28;
inline constexpr int id_h2_channel = 29;
inline constexpr int id_h3_enable = 30;
inline constexpr int id_h3_degree = 31;
inline constexpr int id_h3_octave = 32;
inline constexpr int id_h3_velocity = 33;
inline constexpr int id_h3_follow = 34;
inline constexpr int id_h3_gate = 35;
inline constexpr int id_h3_channel = 36;
inline constexpr int id_s1_active = 37;
inline constexpr int id_s1_mode = 38;
inline constexpr int id_s1_offset = 39;
inline constexpr int id_s1_velocity = 40;
inline constexpr int id_s1_probability = 41;
inline constexpr int id_s1_ratchet = 42;
inline constexpr int id_s1_v1 = 43;
inline constexpr int id_s1_v2 = 44;
inline constexpr int id_s1_v3 = 45;
inline constexpr int id_s1_v4 = 46;
inline constexpr int id_s2_active = 47;
inline constexpr int id_s2_mode = 48;
inline constexpr int id_s2_offset = 49;
inline constexpr int id_s2_velocity = 50;
inline constexpr int id_s2_probability = 51;
inline constexpr int id_s2_ratchet = 52;
inline constexpr int id_s2_v1 = 53;
inline constexpr int id_s2_v2 = 54;
inline constexpr int id_s2_v3 = 55;
inline constexpr int id_s2_v4 = 56;
inline constexpr int id_s3_active = 57;
inline constexpr int id_s3_mode = 58;
inline constexpr int id_s3_offset = 59;
inline constexpr int id_s3_velocity = 60;
inline constexpr int id_s3_probability = 61;
inline constexpr int id_s3_ratchet = 62;
inline constexpr int id_s3_v1 = 63;
inline constexpr int id_s3_v2 = 64;
inline constexpr int id_s3_v3 = 65;
inline constexpr int id_s3_v4 = 66;
inline constexpr int id_s4_active = 67;
inline constexpr int id_s4_mode = 68;
inline constexpr int id_s4_offset = 69;
inline constexpr int id_s4_velocity = 70;
inline constexpr int id_s4_probability = 71;
inline constexpr int id_s4_ratchet = 72;
inline constexpr int id_s4_v1 = 73;
inline constexpr int id_s4_v2 = 74;
inline constexpr int id_s4_v3 = 75;
inline constexpr int id_s4_v4 = 76;
inline constexpr int id_s5_active = 77;
inline constexpr int id_s5_mode = 78;
inline constexpr int id_s5_offset = 79;
inline constexpr int id_s5_velocity = 80;
inline constexpr int id_s5_probability = 81;
inline constexpr int id_s5_ratchet = 82;
inline constexpr int id_s5_v1 = 83;
inline constexpr int id_s5_v2 = 84;
inline constexpr int id_s5_v3 = 85;
inline constexpr int id_s5_v4 = 86;
inline constexpr int id_s6_active = 87;
inline constexpr int id_s6_mode = 88;
inline constexpr int id_s6_offset = 89;
inline constexpr int id_s6_velocity = 90;
inline constexpr int id_s6_probability = 91;
inline constexpr int id_s6_ratchet = 92;
inline constexpr int id_s6_v1 = 93;
inline constexpr int id_s6_v2 = 94;
inline constexpr int id_s6_v3 = 95;
inline constexpr int id_s6_v4 = 96;
inline constexpr int id_s7_active = 97;
inline constexpr int id_s7_mode = 98;
inline constexpr int id_s7_offset = 99;
inline constexpr int id_s7_velocity = 100;
inline constexpr int id_s7_probability = 101;
inline constexpr int id_s7_ratchet = 102;
inline constexpr int id_s7_v1 = 103;
inline constexpr int id_s7_v2 = 104;
inline constexpr int id_s7_v3 = 105;
inline constexpr int id_s7_v4 = 106;
inline constexpr int id_s8_active = 107;
inline constexpr int id_s8_mode = 108;
inline constexpr int id_s8_offset = 109;
inline constexpr int id_s8_velocity = 110;
inline constexpr int id_s8_probability = 111;
inline constexpr int id_s8_ratchet = 112;
inline constexpr int id_s8_v1 = 113;
inline constexpr int id_s8_v2 = 114;
inline constexpr int id_s8_v3 = 115;
inline constexpr int id_s8_v4 = 116;

inline int indexOf (const char* id) noexcept
{
    for (int i = 0; i < kNumParameters; ++i)
        if (std::strcmp (kParams[static_cast<std::size_t> (i)].id, id) == 0)
            return i;
    return -1;
}

} // namespace heptachord::params
