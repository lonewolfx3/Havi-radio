cmake_minimum_required(VERSION 3.22)
if(APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS")
endif()
project(RadioBox VERSION 0.3.0 LANGUAGES C CXX)
if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(JUCE_DIR "" CACHE PATH "Path to a JUCE 8 source checkout")
if(JUCE_DIR)
  add_subdirectory("${JUCE_DIR}" JUCE)
else()
  include(FetchContent)
  FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.6 GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(JUCE)
endif()
juce_add_plugin(RadioBox
  COMPANY_NAME "UOC SOUND"
  BUNDLE_ID "com.havilegrand.haviradio"
  PLUGIN_MANUFACTURER_CODE Havi
  PLUGIN_CODE HvRd
  FORMATS VST3
  PRODUCT_NAME "RadioBox"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE
  VST3_CATEGORIES Fx Filter Distortion)
juce_add_binary_data(RadioBoxArtwork SOURCES Assets/RadioBox.jpg Assets/Gold.jpg Assets/Red.jpg Assets/Burgundy.jpg)
juce_generate_juce_header(RadioBox)
target_sources(RadioBox PRIVATE Source/Plugin.cpp Source/RadioEngine.h)
target_compile_definitions(RadioBox PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(RadioBox PRIVATE RadioBoxArtwork juce::juce_audio_utils PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

enable_testing()
add_executable(RadioBoxDSPTests tests/dsp_test.cpp)
add_test(NAME RadioDSP COMMAND RadioBoxDSPTests)

juce_add_console_app(RadioBoxIntegrationTests PRODUCT_NAME "RadioBox Tests")
juce_generate_juce_header(RadioBoxIntegrationTests)
target_sources(RadioBoxIntegrationTests PRIVATE tests/plugin_test.cpp)
target_compile_definitions(RadioBoxIntegrationTests PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_MODAL_LOOPS_PERMITTED=1 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(RadioBoxIntegrationTests PRIVATE RadioBoxArtwork juce::juce_audio_utils PUBLIC juce::juce_recommended_config_flags)
add_test(NAME RadioBoxIntegration COMMAND RadioBoxIntegrationTests)
