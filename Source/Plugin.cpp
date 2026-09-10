#include <JuceHeader.h>
#include "RadioEngine.h"
#include "BinaryData.h"
#include "Meter.h"
#include "PresetSettings.h"

class HaviRadio final : public juce::AudioProcessor, private juce::AudioProcessorValueTreeState::Listener {
  std::atomic<bool> restoring{false};
  void parameterChanged(const juce::String& id,float value) override {
    if(id=="preset" && !restoring.load()) loadSettings(juce::roundToInt(value));
  }
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
    p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"colour",1},"Interface colour",juce::StringArray{"Original / Olive Green","Gold","Red","Burgundy"},0));
    return p;
  }
  HaviRadio():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"HaviRadio",layout()) {
    loadSettings(0); state.addParameterListener("preset",this);
  }
  ~HaviRadio() override {state.removeParameterListener("preset",this);}
  void loadSettings(int index) {
    const auto& s=havi::settings[size_t(juce::jlimit(0,9,index))];
    const char* ids[]={"bandwidth","tone","grit","noise","mix","output","am","fm","sw"};
    const float values[]={s.bandwidth,s.tone,s.grit,s.noise,s.mix,s.output,float(s.am),float(s.fm),float(s.sw)};
    for(size_t i=0;i<std::size(ids);++i) {
      auto* parameter=state.getParameter(ids[i]);
      parameter->setValueNotifyingHost(parameter->convertTo0to1(values[i]));
    }
  }
  void selectPreset(int index) {
    index=juce::jlimit(0,9,index);
    auto* parameter=state.getParameter("preset");
    parameter->beginChangeGesture();
    if(getCurrentProgram()==index) loadSettings(index);
    else parameter->setValueNotifyingHost(parameter->convertTo0to1(float(index)));
    parameter->endChangeGesture();
  }
  const juce::String getName() const override { return "RadioBox"; }
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
  int getNumPrograms() override {return 10;}
  int getCurrentProgram() override {return juce::roundToInt(state.getRawParameterValue("preset")->load());}
  void setCurrentProgram(int index) override {selectPreset(index);}
  const juce::String getProgramName(int index) override {return havi::presets[size_t(juce::jlimit(0,9,index))].name;}
  void changeProgramName(int,const juce::String&) override {}
  bool hasEditor() const override {return true;}
  juce::AudioProcessorEditor* createEditor() override;
  void getStateInformation(juce::MemoryBlock& dest) override {
    auto xml=state.copyState().createXml(); copyXmlToBinary(*xml,dest);
  }
  void setStateInformation(const void* data,int size) override {
    if(auto xml=getXmlFromBinary(data,size)) if(xml->hasTagName(state.state.getType())) {
      restoring.store(true);
      state.replaceState(juce::ValueTree::fromXml(*xml));
      restoring.store(false);
    }
  }
};

class HardwareLook final: public juce::LookAndFeel_V4 {
public:
  juce::Colour knobColour{0xff363e30};
  void drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float position,float start,float end,juce::Slider& slider) override {
    position=float(slider.getProperties().getWithDefault("displayPosition",position));
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
    g.setGradientFill(juce::ColourGradient(knobColour.brighter(.15f),cx-r*.4f,cy-r*.6f,knobColour.darker(.8f),cx+r*.5f,cy+r,false));
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
  std::array<juce::Image,4> panels{{
    juce::ImageCache::getFromMemory(BinaryData::RadioBox_jpg,BinaryData::RadioBox_jpgSize),
    juce::ImageCache::getFromMemory(BinaryData::Gold_jpg,BinaryData::Gold_jpgSize),
    juce::ImageCache::getFromMemory(BinaryData::Red_jpg,BinaryData::Red_jpgSize),
    juce::ImageCache::getFromMemory(BinaryData::Burgundy_jpg,BinaryData::Burgundy_jpgSize)}};
  juce::ComboBox station,colour;
  juce::TextButton previous{"<"},next{">"},reset{"Reload"};
  std::array<juce::Slider,6> knobs;
  std::array<juce::ToggleButton,3> switches;
  using SliderAttachment=juce::AudioProcessorValueTreeState::SliderAttachment;
  using ButtonAttachment=juce::AudioProcessorValueTreeState::ButtonAttachment;
  std::array<std::unique_ptr<SliderAttachment>,6> attachments;
  std::array<std::unique_ptr<ButtonAttachment>,3> buttonAttachments;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> stationAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> colourAttachment;
  juce::TooltipWindow tooltips{this,500};
  float meterPosition=0;
  int displayedColour=-1;
  juce::Rectangle<int> scaled(int x,int y,int w,int h) const {
    const float sx=float(getWidth())/1536,sy=float(getHeight())/857;
    return {juce::roundToInt(float(x)*sx),juce::roundToInt(float(y)*sy),juce::roundToInt(float(w)*sx),juce::roundToInt(float(h)*sy)};
  }
  void timerCallback() override {
    meterPosition=havi::Meter::animate(meterPosition,processor.meter.take(),1.f/60);
    const juce::Colour colours[]={juce::Colour(0xff363e30),juce::Colour(0xff806122),juce::Colour(0xff861e19),juce::Colour(0xff4b101c)};
    look.knobColour=colours[juce::jlimit(0,3,colour.getSelectedItemIndex())];
    if(displayedColour!=colour.getSelectedItemIndex()) {displayedColour=colour.getSelectedItemIndex();resized();}
    for(auto& k:knobs) {
      const float target=float(k.valueToProportionOfLength(k.getValue()));
      const float current=float(k.getProperties().getWithDefault("displayPosition",target));
      k.getProperties().set("displayPosition",current+.35f*(target-current));
    }
    repaint();
  }
public:
  explicit RadioEditor(HaviRadio& p):AudioProcessorEditor(p),processor(p) {
    setLookAndFeel(&look);
    for(int i=0;i<10;++i) station.addItem(havi::presets[size_t(i)].name,i+1);
    station.setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xff302b1e));
    station.setColour(juce::ComboBox::textColourId,juce::Colour(0xffffdda0));
    station.setTooltip("Select a station, or turn the large tuning dial");
    addAndMakeVisible(station);
    stationAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,"preset",station);
    for(auto* b:{&previous,&next,&reset}) {addAndMakeVisible(b);b->setColour(juce::TextButton::buttonColourId,juce::Colour(0xff302b1e));}
    previous.setName("Previous preset");next.setName("Next preset");reset.setTooltip("Reload the factory settings for this preset");
    previous.onClick=[this]{processor.selectPreset((processor.getCurrentProgram()+9)%10);};
    next.onClick=[this]{processor.selectPreset((processor.getCurrentProgram()+1)%10);};
    reset.onClick=[this]{processor.selectPreset(processor.getCurrentProgram());};
    colour.addItemList(juce::StringArray{"Original / Olive Green","Gold","Red","Burgundy"},1);
    colour.setName("Interface colour");addAndMakeVisible(colour);
    colour.setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xff302b1e));
    colour.setColour(juce::ComboBox::textColourId,juce::Colour(0xffffdda0));
    colourAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,"colour",colour);
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
    g.drawImage(panels[size_t(juce::jlimit(0,3,colour.getSelectedItemIndex()))],juce::Rectangle<float>(0,0,1536,857));
    g.setColour(juce::Colour(0xff121311));g.fillRect(0,0,1536,49);
    g.setColour(juce::Colour(0xffe6d2ae));g.setFont(19.f);
    g.drawText("RadioBox  /  UOC SOUND",26,8,330,32,juce::Justification::centredLeft);
    // A live input scale replaces the printed central needle, retaining the bezel.
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xffffdd95),770,181,juce::Colour(0xffad7735),770,435,false));
    g.fillRoundedRectangle(545,181,449,253,18);
    g.setColour(juce::Colour(0xff503b21));g.setFont(16.f);
    g.drawText("INPUT LEVEL",572,207,390,24,juce::Justification::centred);
    for(int i=0;i<=24;++i) {
      const float x=575+float(i)*16;
      g.drawLine(x,305,x,i%4==0?324.f:315.f,1.5f);
      if(i%4==0) g.drawText(juce::String(-48+i*2),int(x)-18,333,36,20,juce::Justification::centred);
    }
    g.drawText("dBFS",721,376,96,21,juce::Justification::centred);
    const float needleX=575+384*meterPosition;
    g.setColour(juce::Colour(0xffedbe62).withAlpha(.6f));g.fillRect(needleX-4,239.f,8.f,131.f);
    g.setColour(juce::Colour(0xff663721));g.fillRect(needleX-1,239.f,2.f,131.f);
    // Replace the baked meter face so only the live needle remains visible.
    {
    juce::Graphics::ScopedSaveState meterSave(g);
    if(colour.getSelectedItemIndex()==1) g.addTransform(juce::AffineTransform::translation(-26.f,0.f));
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
    }
    // Cover baked status legends/glow beneath all three buttons.
    g.setColour(juce::Colour(0xff24271f));g.fillRoundedRectangle(166,711,260,36,5);
    g.fillRoundedRectangle(166,559,260,23,4);
    for(size_t i=0;i<switches.size();++i) {
      g.setColour(switches[i].getToggleState()?juce::Colour(0xffffbe64):juce::Colour(0xff85836b));
      g.setFont(15.f);g.drawText(switches[i].getToggleState()?"ON":"OFF",174+int(i)*85,716,70,22,juce::Justification::centred);
      g.drawText(switches[i].getToggleState()?"ON":"OFF",174+int(i)*85,559,70,22,juce::Justification::centred);
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
    if(colour.getSelectedItemIndex()==1) {
      knobs[1].setBounds(scaled(1074,180,108,108));
      knobs[3].setBounds(scaled(1275,391,108,108));
    }
    previous.setBounds(scaled(386,8,42,32));
    station.setBounds(scaled(436,8,400,32));
    next.setBounds(scaled(844,8,42,32));
    reset.setBounds(scaled(898,8,85,32));
    colour.setBounds(scaled(1190,8,318,32));
  }
};
juce::AudioProcessorEditor* HaviRadio::createEditor(){return new RadioEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new HaviRadio();}
