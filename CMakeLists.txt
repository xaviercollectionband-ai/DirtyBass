cmake_minimum_required(VERSION 3.22)
project(DirtyBass VERSION 0.1.0)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 9.0.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(DirtyBass
    COMPANY_NAME "Xavi"
    COMPANY_WEBSITE "https://github.com"
    COMPANY_EMAIL ""
    BUNDLE_ID "com.xavi.dirtybass"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Xavi
    PLUGIN_CODE Dbas
    FORMATS VST3
    PRODUCT_NAME "DirtyBass"
)

target_sources(DirtyBass PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

target_compile_features(DirtyBass PRIVATE cxx_std_17)
target_link_libraries(DirtyBass PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
)

juce_generate_juce_header(DirtyBass)
