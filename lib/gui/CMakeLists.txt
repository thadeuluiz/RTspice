juce_add_plugin(rtspice
  COMPANY_NAME RTspice
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE
  PLUGIN_MANUFACTURER_CODE RtSp
  PLUGIN_CODE RtS1
  FORMATS VST3
  PRODUCT_NAME "RTspice")

target_sources(rtspice
  PRIVATE
    include/plugin_editor.hpp
    include/plugin_processor.hpp
    src/plugin_editor.cpp
    src/plugin_processor.cpp)

target_include_directories(rtspice
  PRIVATE
    include/)

target_link_libraries(rtspice
  PRIVATE
    circuit
    parser
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)

target_compile_definitions(rtspice
  PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0)
