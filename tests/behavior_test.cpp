#ifdef NDEBUG
#undef NDEBUG
#endif
#include "../Source/Editor.h"
#include <cassert>
#include <iostream>
static juce::Slider& slider(RadioBoxEditor& e,const char* id){auto* c=dynamic_cast<juce::Slider*>(e.findChildWithID(juce::String("knob.")+id));assert(c);return *c;}
int main(){juce::ScopedJuceInitialiser_GUI init;RadioBoxProcessor p;RadioBoxEditor e(p);juce::AudioBuffer<float> audio(2,512);juce::MidiBuffer midi;
 for(int n=0;n<10;++n){p.selectPreset(n);e.refresh(0);for(size_t i=0;i<havi::controlIDs.size();++i){float actual=p.state.getRawParameterValue(havi::controlIDs[i])->load();assert(std::abs(actual-havi::factoryControls[size_t(n)][i])<.005f);if(i<10)assert(std::abs(slider(e,havi::controlIDs[i]).getValue()-actual)<.005);}}
 auto* menu=dynamic_cast<juce::ComboBox*>(e.findChildWithID("preset.selector"));assert(menu);menu->setSelectedId(3,juce::sendNotificationSync);assert(p.getCurrentProgram()==2);dynamic_cast<juce::TextButton*>(e.findChildWithID("preset.next"))->onClick();assert(p.getCurrentProgram()==3);dynamic_cast<juce::TextButton*>(e.findChildWithID("preset.previous"))->onClick();assert(p.getCurrentProgram()==2);
 for(int i=0;i<90;++i)e.refresh(1.f/60);auto& drive=static_cast<RadioKnob&>(slider(e,"drive"));float before=drive.visualPosition;p.selectPreset(6);e.refresh(0);float target=p.state.getParameter("drive")->convertTo0to1(16.f);assert(drive.visualPosition==before);e.refresh(1.f/60);assert(drive.visualPosition<before&&drive.visualPosition>target);slider(e,"drive").setValue(73.4,juce::sendNotificationSync);assert(std::abs(p.controls()[6]-73.4f)<.01f&&p.modified());
 p.selectBand(2);assert(p.controls()[11]==0&&p.controls()[12]==0&&p.controls()[13]==1);p.selectBand(0);assert(p.controls()[11]==1&&p.controls()[12]==0&&p.controls()[13]==0);
 p.prepareToPlay(48000,512);for(int b=0;b<100;++b){for(int c=0;c<2;++c)for(int i=0;i<512;++i)audio.setSample(c,i,.8f*std::sin(float(i)*.13f));p.processBlock(audio,midi);e.refresh(1.f/60);}assert(e.meterValue()>.8f);for(int b=0;b<180;++b){audio.clear();p.processBlock(audio,midi);e.refresh(1.f/60);}assert(e.meterValue()<.001f);
 auto out=juce::File::getCurrentWorkingDirectory().getChildFile("ui-previews");out.createDirectory();auto* themes=dynamic_cast<juce::ComboBox*>(e.findChildWithID("theme.selector"));assert(themes);const char* names[]={"olive","gold","red","burgundy","blue"};for(int t=0;t<5;++t){themes->setSelectedId(t+1,juce::sendNotificationSync);e.refresh(0);auto pic=e.createComponentSnapshot(e.getLocalBounds());auto stream=out.getChildFile(juce::String(names[t])+".png").createOutputStream();assert(stream);juce::PNGImageFormat f;assert(f.writeImageToStream(pic,*stream));}
 std::cout<<"PASS: ten knobs, ten presets, exclusive bands, live meter and five UI colors.\n";}
