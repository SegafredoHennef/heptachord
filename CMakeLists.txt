cmake_minimum_required(VERSION 3.28)

project(Heptachord VERSION 0.1.7 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# MSVC liest BOM-lose Quellen als Windows-Codepage. Ohne /utf-8 wuerden
# Umlaute und Sonderzeichen in den Labels (ParameterTable.h) zerstoert.
if (MSVC)
    add_compile_options(/utf-8)
endif()

add_subdirectory(extern/JUCE)

# --- Core: Domaenenlogik ohne JUCE (ADR 0007) -------------------------------
add_library(heptachord_core STATIC
    src/core/Scales.cpp
    src/core/StepClock.cpp
    src/core/StepSequencer.cpp
    src/core/StepPlanner.cpp
    src/core/NoteTracker.cpp
    src/core/StepEngine.cpp
    src/core/MidiClock.cpp
    src/core/SettingsFromValues.cpp
    src/core/MidiRender.cpp
)
target_include_directories(heptachord_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(heptachord_core PUBLIC cxx_std_20)

if (MSVC)
    target_compile_options(heptachord_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(heptachord_core PRIVATE -Wall -Wextra)
endif()

# --- Plugin (Instrument, stiller Bus) + Standalone --------------------------
# VST3 nach dem Build in den Systemordner kopieren (braucht Adminrechte).
option(HEPTACHORD_COPY_PLUGIN "VST3 nach dem Build nach Program Files kopieren" OFF)

juce_add_plugin(heptachord
    COMPANY_NAME             "Blakkmoon Audio"
    BUNDLE_ID                "com.blakkmoon.heptachord"
    PLUGIN_MANUFACTURER_CODE Blkk
    PLUGIN_CODE              Hptc
    PRODUCT_NAME             "Heptachord"
    FORMATS                  VST3 Standalone
    IS_SYNTH                 TRUE
    IS_MIDI_EFFECT           FALSE
    NEEDS_MIDI_INPUT         TRUE
    NEEDS_MIDI_OUTPUT        TRUE
    VST3_CATEGORIES          Instrument Synth
    COPY_PLUGIN_AFTER_BUILD  ${HEPTACHORD_COPY_PLUGIN}
)

target_sources(heptachord PRIVATE
    src/plugin/ParameterLayout.cpp
    src/plugin/ParameterBinding.cpp
    src/plugin/ProcessorBridge.cpp
    src/plugin/TestSynth.cpp
    src/plugin/LookAndFeel.cpp
    src/plugin/PluginProcessor.cpp
    src/plugin/PluginEditor.cpp
    src/plugin/Preset.cpp
    src/plugin/ComputerKeyboard.cpp
    src/plugin/MidiExport.cpp
    src/plugin/AudioExport.cpp
    src/plugin/InstrumentHost.cpp
)

target_link_libraries(heptachord
    PRIVATE
        heptachord_core
        juce::juce_audio_utils
        juce::juce_audio_plugin_client
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(heptachord PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_PLUGINHOST_VST3=1
)

# --- Tests ------------------------------------------------------------------
include(FetchContent)

FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.16.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(Catch2)

enable_testing()

add_executable(heptachord_tests
    tests/test_scales.cpp
    tests/test_parameters.cpp
    tests/test_notenames.cpp
    tests/test_stepclock.cpp
    tests/test_sequencer.cpp
    tests/test_planner.cpp
    tests/test_notetracker.cpp
    tests/test_engine.cpp
    tests/test_midiclock.cpp
    tests/test_settings.cpp
    tests/test_bridge.cpp
    tests/test_synth.cpp
    tests/test_preset.cpp
    tests/test_computerkeyboard.cpp
    tests/test_midirender.cpp
    tests/test_midiexport.cpp
    tests/test_audioexport.cpp
    tests/test_recorder.cpp
    src/plugin/ProcessorBridge.cpp
    src/plugin/TestSynth.cpp
    src/plugin/Preset.cpp
    src/plugin/ComputerKeyboard.cpp
    src/plugin/MidiExport.cpp
    src/plugin/AudioExport.cpp
)
target_link_libraries(heptachord_tests PRIVATE
    heptachord_core
    Catch2::Catch2WithMain
    juce::juce_audio_basics
    juce::juce_audio_formats
)
target_compile_definitions(heptachord_tests PRIVATE
    HEPTACHORD_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    HEPTACHORD_PRESETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/presets"
    JUCE_PLUGINHOST_VST3=1
)

# Factory-Presets in den Build-Ordner kopieren, damit der Editor sie laden kann.
add_custom_command(TARGET heptachord POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/presets"
            "$<TARGET_FILE_DIR:heptachord>/presets"
    COMMENT "Kopiere Factory-Presets in den Build-Ordner"
)

add_test(NAME core COMMAND heptachord_tests)
