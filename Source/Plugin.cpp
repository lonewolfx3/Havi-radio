#include "Editor.h"
juce::AudioProcessorEditor* RadioBoxProcessor::createEditor(){return new RadioBoxEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new RadioBoxProcessor();}
