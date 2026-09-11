cmake_minimum_required(VERSION 3.22)
if(APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS")
endif()
project(HaviRadio VERSION 0.4.0 LANGUAGES C CXX)
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
juce_add_plugin(HaviRadio
  COMPANY_NAME "UOC SOUND"
  BUNDLE_ID "com.uocsound.radiobox"
  PLUGIN_MANUFACTURER_CODE Uocs
  PLUGIN_CODE RdBx
  FORMATS VST3
  PRODUCT_NAME "RadioBox"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE
  VST3_CATEGORIES Fx Filter Distortion)
file(GLOB RADIOBOX_LAYERED_ARTWORK CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Layered/*")
juce_add_binary_data(RadioBoxArtwork SOURCES ${RADIOBOX_LAYERED_ARTWORK})
juce_generate_juce_header(HaviRadio)
target_sources(HaviRadio PRIVATE Source/Plugin.cpp Source/RadioEngine.h)
target_compile_definitions(HaviRadio PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(HaviRadio PRIVATE RadioBoxArtwork juce::juce_audio_utils PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

enable_testing()
add_executable(HaviRadioDSPTests tests/dsp_test.cpp)
add_test(NAME RadioDSP COMMAND HaviRadioDSPTests)

add_executable(RadioBoxBehaviorTests tests/behavior_test.cpp)
target_link_libraries(RadioBoxBehaviorTests PRIVATE HaviRadio RadioBoxArtwork juce::juce_audio_utils)
target_include_directories(RadioBoxBehaviorTests PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/HaviRadio_artefacts/JuceLibraryCode")
add_test(NAME RadioBoxBehavior COMMAND RadioBoxBehaviorTests)
