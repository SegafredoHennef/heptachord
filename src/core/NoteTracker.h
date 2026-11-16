#pragma once

#include <array>

namespace heptachord
{

// Eingangsseite: monophon mit Last-Note-Priority und Latch
// (docs/03-spezifikation.md 2.1, 2.2).
//
// Die gehaltenen Noten liegen als Stapel vor; die zuletzt gedrueckte Note
// gewinnt. Wird sie losgelassen, faellt der Tracker auf die naechste gehaltene
// Note zurueck.
class NoteTracker
{
public:
    void setLatch (bool latch) noexcept;

    // Note-On ersetzt eine bereits gehaltene gleiche Note (kein Doppel im Stapel).
    void noteOn (int pitch, int velocity) noexcept;
    void noteOff (int pitch) noexcept;

    // Laesst alle Tasten los, ohne den Latch-Speicher zu loeschen
    // (Transport-Stop).
    void releaseAll() noexcept;

    // Loescht auch den Latch-Speicher (reset, Preset-Wechsel).
    void reset() noexcept;

    bool active() const noexcept;
    int pitch() const noexcept;
    int velocity() const noexcept;

    int heldCount() const noexcept { return m_count; }
    bool hasLatch() const noexcept { return m_hasLatch; }

private:
    void remove (int pitch) noexcept;

    std::array<int, 128> m_held {};
    int m_count = 0;
    int m_lastPitch = 60;
    int m_lastVelocity = 100;
    bool m_latch = false;
    bool m_hasLatch = false;   // Latch-Speicher gefuellt
};

} // namespace heptachord
