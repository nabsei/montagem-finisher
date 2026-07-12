cmake_minimum_required(VERSION 3.22)
project(MontagemFinisher VERSION 0.1.0)

add_subdirectory(libs/JUCE)

if(APPLE)
    set(PHONK_FORMATS VST3 AU Standalone)
else()
    set(PHONK_FORMATS VST3 Standalone)
endif()

juce_add_plugin(MontagemFinisher
    COMPANY_NAME "montagem.finisher"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Mtgf
    PLUGIN_CODE Fnsh
    FORMATS ${PHONK_FORMATS}
    PRODUCT_NAME "Montagem Finisher")

target_sources(MontagemFinisher PRIVATE
    Source/FinisherProcessor.cpp
    Source/PluginEditor.cpp
    Source/PluginEntry.cpp)

target_compile_definitions(MontagemFinisher PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(MontagemFinisher
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
