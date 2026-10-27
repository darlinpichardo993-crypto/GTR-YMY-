cmake_minimum_required(VERSION 3.22)
project(ORYMY_Bass VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)
juce_add_plugin(ORYMYBass
  COMPANY_NAME "YMY Sounds"
  BUNDLE_ID "com.ymysounds.orymybass"
  PLUGIN_MANUFACTURER_CODE Ymyr
  PLUGIN_CODE ORBs
  FORMATS VST3 Standalone
  PRODUCT_NAME "ORYMY Bass"
  DESCRIPTION "Dominican bass amp and tone suite"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS TRUE
  COPY_PLUGIN_AFTER_BUILD FALSE
)
juce_generate_juce_header(ORYMYBass)
target_sources(ORYMYBass PRIVATE PluginProcessor.cpp PluginEditor.cpp)
target_compile_definitions(ORYMYBass PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(ORYMYBass PRIVATE juce::juce_audio_utils juce::juce_dsp juce::juce_gui_basics PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
