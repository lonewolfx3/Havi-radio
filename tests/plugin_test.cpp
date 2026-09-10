#ifdef NDEBUG
#undef NDEBUG
#endif
#include "../Source/Plugin.cpp"
#include <cassert>
#include <iostream>

static void set(HaviRadio& p,const char* id,float value) {
  auto* parameter=p.state.getParameter(id);
  parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
}
static float get(HaviRadio& p,const char* id) {return p.state.getRawParameterValue(id)->load();}
static void near(float a,float b) {assert(std::abs(a-b)<.001f);}
int main() {
  juce::ScopedJuceInitialiser_GUI gui;
  HaviRadio p;
  p.prepareToPlay(48000,512);
  assert(p.getName()=="RadioBox");
  std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
  for(int index=0;index<10;++index) {
    // Exercise the host-automatable station parameter as well as host programs.
    set(p,"preset",float((index+1)%10));p.setCurrentProgram(index);
    const auto& s=havi::settings[size_t(index)];
    near(get(p,"tone"),s.tone);near(get(p,"bandwidth"),s.bandwidth);
    near(get(p,"grit"),s.grit);near(get(p,"noise"),s.noise);
    near(get(p,"mix"),s.mix);near(get(p,"output"),s.output);
    near(get(p,"am"),float(s.am));near(get(p,"fm"),float(s.fm));near(get(p,"sw"),float(s.sw));
    juce::MessageManager::getInstance()->runDispatchLoopUntil(30);
    for(auto* child:editor->getChildren()) if(auto* slider=dynamic_cast<juce::Slider*>(child)) {
      const auto name=slider->getName();
      if(name=="Drive") near(float(slider->getValue()),s.grit);
      if(name=="Bandwidth") near(float(slider->getValue()),s.bandwidth);
      if(name=="Tuning") near(float(slider->getValue()),float(index));
    }
  }
  set(p,"tone",.73f);set(p,"mix",.44f);set(p,"am",0);set(p,"colour",3);
  juce::MemoryBlock saved;p.getStateInformation(saved);
  HaviRadio restored;restored.setStateInformation(saved.getData(),int(saved.getSize()));
  near(get(restored,"tone"),.73f);near(get(restored,"mix"),.44f);
  near(get(restored,"am"),0);near(get(restored,"colour"),3);near(get(restored,"preset"),9);
  restored.selectPreset(9);near(get(restored,"tone"),havi::settings[9].tone);
  // Palette changes must leave the sound settings alone.
  for(int c=0;c<4;++c) {set(p,"colour",float(c));near(get(p,"tone"),.73f);near(get(p,"mix"),.44f);}
  juce::AudioBuffer<float> audio(2,512);juce::MidiBuffer midi;
  audio.clear();p.processBlock(audio,midi);near(p.meter.take(),0);
  for(int c=0;c<2;++c) for(int i=0;i<512;++i) audio.setSample(c,i,.5f);
  p.processBlock(audio,midi);near(p.meter.take(),.5f);
  const auto folder=juce::File::getCurrentWorkingDirectory().getChildFile("previews");folder.createDirectory();
  const char* names[]={"Olive","Gold","Red","Burgundy"};
  for(int c=0;c<4;++c) {
    set(p,"colour",float(c));p.selectPreset(6);
    juce::MessageManager::getInstance()->runDispatchLoopUntil(180);
    auto shot=editor->createComponentSnapshot(editor->getLocalBounds());
    auto out=folder.getChildFile(juce::String(names[c])+".png").createOutputStream();
    assert(out && juce::PNGImageFormat().writeImageToStream(shot,*out));
  }
  std::cout<<"PASS: 10 factory presets, UI bindings, manual edits, session recall, palette isolation, live meter, four UI previews.\n";
}
