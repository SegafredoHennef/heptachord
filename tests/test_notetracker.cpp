#include "core/NoteTracker.h"

#include <catch2/catch_test_macros.hpp>

using namespace heptachord;

TEST_CASE ("Letzte Note gewinnt", "[notetracker]")
{
    NoteTracker tracker;

    REQUIRE_FALSE (tracker.active());

    tracker.noteOn (60, 100);
    REQUIRE (tracker.active());
    REQUIRE (tracker.pitch() == 60);
    REQUIRE (tracker.velocity() == 100);

    tracker.noteOn (64, 90);
    REQUIRE (tracker.pitch() == 64);
    REQUIRE (tracker.velocity() == 90);
    REQUIRE (tracker.heldCount() == 2);

    // Loslassen der letzten Note faellt auf die vorherige zurueck.
    tracker.noteOff (64);
    REQUIRE (tracker.active());
    REQUIRE (tracker.pitch() == 60);
    REQUIRE (tracker.velocity() == 100);
    REQUIRE (tracker.heldCount() == 1);

    tracker.noteOff (60);
    REQUIRE_FALSE (tracker.active());
}

TEST_CASE ("Dieselbe Note doppelt gehalten", "[notetracker]")
{
    NoteTracker tracker;

    tracker.noteOn (60, 100);
    tracker.noteOn (60, 100);
    REQUIRE (tracker.heldCount() == 1);

    tracker.noteOff (60);
    REQUIRE_FALSE (tracker.active());
}

TEST_CASE ("Latch haelt die letzte Note", "[notetracker]")
{
    NoteTracker tracker;
    tracker.setLatch (true);

    tracker.noteOn (67, 110);
    tracker.noteOff (67);

    REQUIRE (tracker.active());
    REQUIRE (tracker.pitch() == 67);
    REQUIRE (tracker.velocity() == 110);
    REQUIRE (tracker.heldCount() == 0);

    // Latch aus loescht den Speicher.
    tracker.setLatch (false);
    REQUIRE_FALSE (tracker.active());
}

TEST_CASE ("releaseAll behaelt den Latch, reset nicht", "[notetracker]")
{
    NoteTracker tracker;
    tracker.setLatch (true);
    tracker.noteOn (62, 100);
    tracker.noteOff (62);

    tracker.releaseAll();
    REQUIRE (tracker.active());
    REQUIRE (tracker.pitch() == 62);

    tracker.reset();
    REQUIRE_FALSE (tracker.active());
}

TEST_CASE ("Neue Note ueberschreibt den Latch", "[notetracker]")
{
    NoteTracker tracker;
    tracker.setLatch (true);

    tracker.noteOn (60, 100);
    tracker.noteOff (60);
    tracker.noteOn (72, 80);

    REQUIRE (tracker.pitch() == 72);
    REQUIRE (tracker.velocity() == 80);
}
