#include <JuceHeader.h>
#include "RadioEngine.h"
#include "BinaryData.h"
#include "Meter.h"

class HaviRadio final : public juce::AudioProcessor {
public:
  juce::AudioProcessorValueTreeState state;
  havi::RadioEngine engine;
  havi::Meter meter;
  static juce::AudioProcessorValueTreeState::ParameterLayout layout() {
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    juce::StringArray names; for(auto& preset:havi::presets) names.add(preset.name);
    p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"preset",1},"Station",names,0));
    auto add=[&](const char* id,const char* name,float min,float max,float value){
      p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},name,juce::NormalisableRange<float>{min,max,.01f},value));
    };
    add("tone","Tone",-1,1,0); add("grit","Grit",0,2,1);
    add("noise","Static",0,2,1); add("mix","Mix",0,1,1); add("output","Output (dB)",-24,6,0);
    add("bandwidth","Bandwidth",-1,1,0);
    for(const auto* id:{"am","fm","sw"}) p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{id,1},juce::String(id).toUpperCase(),true));
    return p;
  }
  HaviRadio():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"HaviRadio",layout()) {}
  const juce::String getName() const override { return "Havi Radio"; }
  void prepareToPlay(double sr,int) override {engine.prepare(sr); meter.reset();}
  void releaseResources() override {}
  bool isBusesLayoutSupported(const BusesLayout& b) const override {
    auto out=b.getMainOutputChannelSet();
    return (out==juce::AudioChannelSet::mono()||out==juce::AudioChannelSet::stereo())&&out==b.getMainInputChannelSet();
  }
  void processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer&) override {
    juce::ScopedNoDenormals guard;
    float inputPeak=0;
    for(int c=0;c<buffer.getNumChannels();++c)
      for(int i=0;i<buffer.getNumSamples();++i) {
        const float sample=buffer.getSample(c,i);
        if(std::isfinite(sample)) inputPeak=std::max(inputPeak,std::abs(sample));
      }
    meter.push(inputPeak);
    auto v=[&](const char* id){return state.getRawParameterValue(id)->load();};
    engine.process(buffer.getArrayOfWritePointers(),buffer.getNumChannels(),buffer.getNumSamples(),int(v("preset")),v("tone"),v("grit"),v("noise"),v("mix"),v("output"),v("bandwidth"),v("am")>.5f,v("fm")>.5f,v("sw")>.5f);
  }
  bool acceptsMidi() const override {return false;}
  bool producesMidi() const override {return false;}
  double getTailLengthSeconds() const override {return .25;}
  int getNumPrograms() override {return 1;}
  int getCurrentProgram() override {return 0;}
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override {return {};}
  void changeProgramName(int,const juce::String&) override {}
  bool hasEditor() const override {return true;}
  juce::AudioProcessorEditor* createEditor() override;
  void getStateInformation(juce::MemoryBlock& dest) override {
    auto xml=state.copyState().createXml(); copyXmlToBinary(*xml,dest);
  }
  void setStateInformation(const void* data,int size) override {
    if(auto xml=getXmlFromBinary(data,size)) if(xml->hasTagName(state.state.getType())) state.replaceState(juce::ValueTree::fromXml(*xml));
  }
};

class HardwareLook final: public juce::LookAndFeel_V4 {
public:
  void drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float position,float start,float end,juce::Slider&) override {
    const float d=float(juce::jmin(w,h))-4, r=d*.5f;
    const float cx=float(x)+float(w)*.5f,cy=float(y)+float(h)*.5f;
    auto circle=juce::Rectangle<float>(cx-r,cy-r,d,d);
    g.setColour(juce::Colour(0xff11130f));g.fillEllipse(circle.expanded(2));
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xffb4ad94),cx-r,cy-r,juce::Colour(0xff1e211b),cx+r,cy+r,false));
    g.fillEllipse(circle);
    g.setColour(juce::Colour(0xff161914));g.fillEllipse(circle.reduced(d*.07f));
    const float angle=start+position*(end-start);
    for(int i=0;i<36;++i) {
      float a=angle+float(i)*juce::MathConstants<float>::twoPi/36;
      g.setColour(i%2?juce::Colour(0xff646654):juce::Colour(0xff30362a));
      g.drawLine(cx+std::sin(a)*r*.86f,cy-std::cos(a)*r*.86f,cx+std::sin(a)*r*.96f,cy-std::cos(a)*r*.96f,1.6f);
    }
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff363e30),cx-r*.4f,cy-r*.6f,juce::Colour(0xff080d09),cx+r*.5f,cy+r,false));
    g.fillEllipse(circle.reduced(d*.17f));
    g.setColour(juce::Colour(0xff7e8068));g.drawEllipse(circle.reduced(d*.17f),1.3f);
    g.setColour(juce::Colour(0xffffdfa1));
    g.drawLine(cx+std::sin(angle)*r*.44f,cy-std::cos(angle)*r*.44f,cx+std::sin(angle)*r*.76f,cy-std::cos(angle)*r*.76f,3.f);
  }
  void drawToggleButton(juce::Graphics& g,juce::ToggleButton& button,bool over,bool down) override {
    const auto b=button.getLocalBounds().toFloat();const bool on=button.getToggleState();
    g.setColour(juce::Colour(0xff24271f));g.fillRoundedRectangle(b,5);
    const auto face=b.reduced(b.getWidth()*.13f,b.getHeight()*.12f);
    if(on) {g.setColour(juce::Colour(0xffed941f).withAlpha(.42f));g.fillRoundedRectangle(face.expanded(6),7);}
    g.setGradientFill(juce::ColourGradient(on?juce::Colour(0xffffcc70):juce::Colour(0xff9b9276),face.getX(),face.getY(),on?juce::Colour(0xffffebad):juce::Colour(0xffb5aa8b),face.getX(),face.getBottom(),false));
    g.fillRoundedRectangle(face.translated(0,down?2.f:0.f),4);
    g.setColour(over?juce::Colour(0xffffe2a2):juce::Colour(0xff625a45));g.drawRoundedRectangle(face,4,2);
    g.setColour(juce::Colour(0xff393422));g.setFont(face.getWidth()*.34f);
    g.drawText("OFF",face.withHeight(face.getHeight()*.35f),juce::Justification::centred);
    g.drawText("ON",face.withY(face.getBottom()-face.getHeight()*.35f).withHeight(face.getHeight()*.35f),juce::Justification::centred);
    g.setColour(on?juce::Colour(0xffffb743):juce::Colour(0xff35382b));
    g.fillEllipse(b.getCentreX()-3,b.getBottom()-8,6,5);
  }
};

class RadioEditor final:public juce::AudioProcessorEditor,private juce::Timer {
  HaviRadio& processor;
  HardwareLook look;
  juce::Image panel=juce::ImageCache::getFromMemory(BinaryData::RadioBox_jpg,BinaryData::RadioBox_jpgSize);
  juce::ComboBox station;
  std::array<juce::Slider,6> knobs;
  std::array<juce::ToggleButton,3> switches;
  using SliderAttachment=juce::AudioProcessorValueTreeState::SliderAttachment;
  using ButtonAttachment=juce::AudioProcessorValueTreeState::ButtonAttachment;
  std::array<std::unique_ptr<SliderAttachment>,6> attachments;
  std::array<std::unique_ptr<ButtonAttachment>,3> buttonAttachments;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> stationAttachment;
  juce::TooltipWindow tooltips{this,500};
  float meterPosition=0;
  juce::Rectangle<int> scaled(int x,int y,int w,int h) const {
    const float sx=float(getWidth())/1536,sy=float(getHeight())/857;
    return {juce::roundToInt(float(x)*sx),juce::roundToInt(float(y)*sy),juce::roundToInt(float(w)*sx),juce::roundToInt(float(h)*sy)};
  }
  void timerCallback() override {meterPosition=havi::Meter::animate(meterPosition,processor.meter.take(),1.f/60);repaint();}
public:
  explicit RadioEditor(HaviRadio& p):AudioProcessorEditor(p),processor(p) {
    setLookAndFeel(&look);
    for(int i=0;i<10;++i) station.addItem(havi::presets[size_t(i)].name,i+1);
    station.setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xff302b1e));
    station.setColour(juce::ComboBox::textColourId,juce::Colour(0xffffdda0));
    station.setTooltip("Select a station, or turn the large tuning dial");
    addAndMakeVisible(station);
    stationAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,"preset",station);
    const char* ids[]={"bandwidth","tone","grit","output","mix","preset"};
    const char* names[]={"Bandwidth","Filter","Drive","Output","Dry / Wet","Tuning"};
    const char* tips[]={"Bandwidth: narrow or widen the radio frequency range","Filter: move the radio tone darker or brighter","Drive: saturation amount; AM enables the grit stage","Output level in dB","Dry / wet balance","Tuning: select one of ten radio presets"};
    for(size_t i=0;i<knobs.size();++i) {
      auto& k=knobs[i];k.setName(names[i]);k.setTooltip(tips[i]);
      k.setSliderStyle(juce::Slider::RotaryVerticalDrag);k.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
      k.setRotaryParameters(juce::MathConstants<float>::pi*1.25f,juce::MathConstants<float>::pi*2.75f,true);
      k.setPopupDisplayEnabled(true,true,this);k.setMouseDragSensitivity(180);
      k.setDoubleClickReturnValue(true,(i==2||i==4)?1:0);
      addAndMakeVisible(k);attachments[i]=std::make_unique<SliderAttachment>(p.state,ids[i],k);
    }
    const char* buttonIds[]={"am","fm","sw"};
    const char* buttonTips[]={"AM: enable or disable saturation","FM: enable or disable wavering movement","SW: enable or disable static"};
    for(size_t i=0;i<switches.size();++i) {
      switches[i].setName(juce::String(buttonIds[i]).toUpperCase());switches[i].setTooltip(buttonTips[i]);
      addAndMakeVisible(switches[i]);buttonAttachments[i]=std::make_unique<ButtonAttachment>(p.state,buttonIds[i],switches[i]);
    }
    setResizable(true,true);getConstrainer()->setFixedAspectRatio(1536.0/857.0);
    setResizeLimits(768,429,1536,857);setSize(1152,643);startTimerHz(60);
  }
  ~RadioEditor() override {stopTimer();setLookAndFeel(nullptr);}
  void paint(juce::Graphics& graphics) override {
    juce::Graphics::ScopedSaveState save(graphics);
    graphics.addTransform(juce::AffineTransform::scale(float(getWidth())/1536,float(getHeight())/857));
    auto& g=graphics;g.fillAll(juce::Colours::black);
    g.drawImage(panel,juce::Rectangle<float>(0,0,1536,857));
    // Replace the baked meter face so only the live needle remains visible.
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xffffd786),1150,390,juce::Colour(0xffc08e42),1150,485,false));
    g.fillRoundedRectangle(1090,389,124,92,28);
    const juce::Point<float> pivot(1151,470);
    for(int i=0;i<=12;++i) {
      const float a=(-58.f+116.f*float(i)/12)*juce::MathConstants<float>::pi/180;
      g.setColour(i>9?juce::Colour(0xffa63922):juce::Colour(0xff493921));
      g.drawLine(pivot.x+std::sin(a)*59,pivot.y-std::cos(a)*59,pivot.x+std::sin(a)*(i%3==0?68:64),pivot.y-std::cos(a)*(i%3==0?68:64),1.5f);
    }
    g.setColour(juce::Colour(0xff45351e));g.setFont(11.f);
    g.drawText("INPUT",1117,428,68,17,juce::Justification::centred);
    g.setFont(8.f);g.drawText("-48       dBFS       0",1097,392,109,13,juce::Justification::centred);
    const float angle=(-58+116*meterPosition)*juce::MathConstants<float>::pi/180;
    g.setColour(juce::Colour(0xff3a2819));g.drawLine(pivot.x,pivot.y,pivot.x+std::sin(angle)*64,pivot.y-std::cos(angle)*64,2);
    g.fillEllipse(pivot.x-7,pivot.y-7,14,14);
    // Cover baked status legends/glow beneath all three buttons.
    g.setColour(juce::Colour(0xff24271f));g.fillRoundedRectangle(166,711,260,36,5);
    for(size_t i=0;i<switches.size();++i) {
      g.setColour(switches[i].getToggleState()?juce::Colour(0xffffbe64):juce::Colour(0xff85836b));
      g.setFont(15.f);g.drawText(switches[i].getToggleState()?"ON":"OFF",174+int(i)*85,716,70,22,juce::Justification::centred);
    }
  }
  void resized() override {
    knobs[0].setBounds(scaled(240,267,114,114));
    knobs[1].setBounds(scaled(1099,180,108,108));
    knobs[2].setBounds(scaled(1284,182,108,108));
    knobs[3].setBounds(scaled(1294,391,108,108));
    knobs[4].setBounds(scaled(1189,594,114,114));
    knobs[5].setBounds(scaled(669,545,205,205));
    for(int i=0;i<3;++i) switches[size_t(i)].setBounds(scaled(169+i*85,584,80,126));
    station.setBounds(scaled(573,376,388,43));
  }
};
juce::AudioProcessorEditor* HaviRadio::createEditor(){return new RadioEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new HaviRadio();}
