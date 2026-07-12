cmake_minimum_required(VERSION 3.22)
project(PhonkFinisher VERSION 0.1.0)

add_subdirectory(libs/JUCE)

juce_add_plugin(PhonkFinisher
    COMPANY_NAME "MontagemFinisher"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Mtgf
    PLUGIN_CODE Fnsh
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "Phonk Finisher")

target_sources(PhonkFinisher PRIVATE
    Source/FinisherProcessor.cpp
    Source/PluginEditor.cpp
    Source/PluginEntry.cpp)

target_compile_definitions(PhonkFinisher PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(PhonkFinisher
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
