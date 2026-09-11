#pragma once
#include <JuceHeader.h>
#include "RadioEngine.h"
#include "FactoryPresets.h"
#include "Meter.h"
class RadioBoxProcessor final:public juce::AudioProcessor,private juce::AudioProcessorValueTreeState::Listener {
 std::array<std::atomic<float>*,16> raw{};std::atomic<unsigned> revision{0};std::atomic<bool> restoring{false};std::atomic<int> activeProgram{0};havi::Controls audioControls=havi::factoryControls[0];unsigned audioRevision=~0u;int audioProgram=0;std::array<float,3> dropoutBands{1,0,0};
 void setRaw(const char* id,float value){auto* p=state.getParameter(id);p->setValueNotifyingHost(p->convertTo0to1(value));}
 void saveDropoutBands(){for(int i=0;i<3;++i){dropoutBands[size_t(i)]=raw[size_t(11+i)]->load();state.state.setProperty(juce::Identifier("dropoutBand"+juce::String(i)),dropoutBands[size_t(i)],nullptr);}}
 void parameterChanged(const juce::String& id,float value) override{
  if(restoring.load())return;
  if(id=="preset")applyFactory(juce::jlimit(0,9,int(std::lround(value))));
  else if(id=="dropout"){
   restoring.store(true);revision.fetch_add(1,std::memory_order_acq_rel);
   if(value>.5f){saveDropoutBands();for(int i=0;i<3;++i)setRaw(havi::controlIDs[size_t(11+i)],0);}
   else{for(int i=0;i<3;++i)setRaw(havi::controlIDs[size_t(11+i)],dropoutBands[size_t(i)]);}
   revision.fetch_add(1,std::memory_order_release);restoring.store(false);
  }
 }
 void applyFactory(int index){revision.fetch_add(1,std::memory_order_acq_rel);restoring.store(true);activeProgram.store(index);const auto& c=havi::factoryControls[size_t(index)];for(int i=0;i<3;++i){dropoutBands[size_t(i)]=c[size_t(11+i)];state.state.setProperty(juce::Identifier("dropoutBand"+juce::String(i)),dropoutBands[size_t(i)],nullptr);}for(size_t i=0;i<c.size();++i)setRaw(havi::controlIDs[i],c[14]>.5f&&i>=11&&i<=13?0.f:c[i]);restoring.store(false);revision.fetch_add(1,std::memory_order_release);}
public:
 juce::AudioProcessorValueTreeState state;havi::RadioEngine engine;havi::Meter meter;std::atomic<bool> overload{false};
 static juce::AudioProcessorValueTreeState::ParameterLayout layout(){juce::AudioProcessorValueTreeState::ParameterLayout p;juce::StringArray names;for(auto* n:havi::presetNames)names.add(n);p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"preset",1},"Preset",names,0));
  auto f=[&](const char* id,const char* name,float lo,float hi,float step,float def){p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},name,juce::NormalisableRange<float>{lo,hi,step},def));};
  const auto& d=havi::factoryControls[0];f("loRoll","Lo Roll-Off",20,1200,1,d[0]);f("midEq","Mid EQ",-12,12,.1f,d[1]);f("hiRoll","Hi Roll-Off",700,18000,1,d[2]);f("bandwidth","Bandwidth",0,100,.1f,d[3]);f("tuning","Tuning",-80,100,1,d[4]);f("filter","Filter",0,100,.1f,d[5]);f("drive","Drive",0,100,.1f,d[6]);f("tone","Tone",-1,1,.01f,d[7]);f("width","Stereo Width",0,200,.1f,d[8]);f("mix","Dry / Wet",0,100,.1f,d[9]);f("output","Output",-24,6,.1f,d[10]);
  for(size_t i=11;i<16;++i)p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{havi::controlIDs[i],1},juce::String(havi::controlIDs[i]),d[i]>.5f));
  p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"vuLight",1},"VU Backlight",true));p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"theme",1},"UI Color",juce::StringArray{"Olive Green","Burgundy","Blue"},0));return p;}
 RadioBoxProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"HaviRadio",layout()){for(size_t i=0;i<raw.size();++i)raw[i]=state.getRawParameterValue(havi::controlIDs[i]);state.addParameterListener("preset",this);state.addParameterListener("dropout",this);}
 ~RadioBoxProcessor()override{state.removeParameterListener("preset",this);state.removeParameterListener("dropout",this);}havi::Controls controls()const{havi::Controls c{};for(size_t i=0;i<c.size();++i)c[i]=raw[i]->load();return c;}
 void selectPreset(int i){i=juce::jlimit(0,9,i);auto* p=state.getParameter("preset");if(i==getCurrentProgram()){applyFactory(i);return;}p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(float(i)));p->endChangeGesture();}
 void selectBand(int selected){selected=juce::jlimit(0,2,selected);revision.fetch_add(1);for(int i=0;i<3;++i){auto* p=state.getParameter(havi::controlIDs[size_t(11+i)]);p->setValueNotifyingHost(p->convertTo0to1(i==selected?1.f:0.f));}revision.fetch_add(1);}
 void setDropout(bool enabled){auto* p=state.getParameter("dropout");p->beginChangeGesture();p->setValueNotifyingHost(enabled?1.f:0.f);p->endChangeGesture();}
 void loadUserState(const juce::ValueTree& tree){if(!tree.isValid()||tree.getType()!=state.state.getType())return;restoring.store(true);revision.fetch_add(1,std::memory_order_acq_rel);state.replaceState(tree.createCopy());activeProgram.store(juce::jlimit(0,9,int(state.getRawParameterValue("preset")->load())));for(int i=0;i<3;++i)dropoutBands[size_t(i)]=float(state.state.getProperty(juce::Identifier("dropoutBand"+juce::String(i)),i==1?1.f:0.f));revision.fetch_add(1,std::memory_order_release);restoring.store(false);}
 bool modified()const{return !havi::matchesFactory(controls(),activeProgram.load());}const juce::String getName()const override{return "RadioBox";}void prepareToPlay(double sr,int)override{engine.prepare(sr);meter.reset();overload=false;audioRevision=~0u;}void releaseResources()override{}
 bool isBusesLayoutSupported(const BusesLayout& b)const override{auto o=b.getMainOutputChannelSet();return(o==juce::AudioChannelSet::mono()||o==juce::AudioChannelSet::stereo())&&o==b.getMainInputChannelSet();}
 void processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&)override{juce::ScopedNoDenormals guard;float peak=0;for(int c=0;c<b.getNumChannels();++c)for(int i=0;i<b.getNumSamples();++i){const float x=b.getSample(c,i);if(std::isfinite(x))peak=std::max(peak,std::abs(x));}meter.push(peak);overload.store(peak>=.988f);
  const unsigned before=revision.load(std::memory_order_acquire);if((before&1u)==0){auto candidate=controls();int program=activeProgram.load();if(before==revision.load(std::memory_order_acquire)){audioControls=candidate;audioProgram=program;audioRevision=before;}}
  const auto& v=audioControls;int band=v[11]>.5f?0:v[12]>.5f?1:2;engine.process(b.getArrayOfWritePointers(),b.getNumChannels(),b.getNumSamples(),v[0],v[1],v[2],v[3],v[4],v[5],v[6],v[7],v[8],v[9],v[10],band,v[14]>.5f,v[15]>.5f);
 }
 bool acceptsMidi()const override{return false;}bool producesMidi()const override{return false;}double getTailLengthSeconds()const override{return .25;}int getNumPrograms()override{return 10;}int getCurrentProgram()override{return activeProgram.load();}void setCurrentProgram(int i)override{selectPreset(i);}const juce::String getProgramName(int i)override{return havi::presetNames[size_t(juce::jlimit(0,9,i))];}void changeProgramName(int,const juce::String&)override{}bool hasEditor()const override{return true;}juce::AudioProcessorEditor* createEditor()override;
 void getStateInformation(juce::MemoryBlock& dest)override{auto x=state.copyState().createXml();copyXmlToBinary(*x,dest);}void setStateInformation(const void* data,int size)override{if(auto x=getXmlFromBinary(data,size))if(x->hasTagName(state.state.getType()))loadUserState(juce::ValueTree::fromXml(*x));}
};
