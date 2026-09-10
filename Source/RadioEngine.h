#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace havi {
struct Preset { const char* name; float low, high, drive, flutter, rate, noise; };
inline constexpr std::array<Preset, 10> presets {{
  {"VINTAGE RADIO", 420, 2800, 3.2f, .045f, 5.1f, .010f},
  {"RADIO 2000'S", 220, 5200, 2.0f, .006f, 3.0f, .002f},
  {"80'S RADIO", 170, 6200, 1.8f, .025f, 4.3f, .005f},
  {"SUMMER 2015", 140, 7800, 1.4f, .012f, 1.7f, .001f},
  {"URBAN CITY RADIO", 310, 4000, 4.0f, .008f, 3.8f, .003f},
  {"UNDER WATER RADIO", 90, 850, 1.3f, .35f, .75f, .002f},
  {"MODERN RADIO", 100, 10500, 1.2f, 0, 1, 0},
  {"BOOM BOX RADIO", 150, 3800, 3.7f, .03f, 6.0f, .006f},
  {"TELEPHONE RADIO", 650, 2300, 2.7f, 0, 1, .001f},
  {"MIDNIGHT AM", 550, 1800, 4.5f, .09f, 2.2f, .014f}
}};

// Allocation-free mono/stereo DSP. Two cascaded one-pole sections per edge.
// The distortion is intentionally lo-fi; this prototype is not oversampled.
class RadioEngine {
  struct Channel { float hp1=0, hp2=0, lp1=0, lp2=0, envelope=0; };
  std::array<Channel,2> channels{};
  double sampleRate=48000, phase=0;
  float smoothing=.001f, low=420, high=2800, drive=3.2f;
  float saturation=1;
  float flutter=.045f, rate=5.1f, hiss=.01f, mix=1, output=1;
  uint32_t rng=0x428124ab;
  float smooth(float current,float target) const { return current+smoothing*(target-current); }
public:
  void prepare(double sr) {
    sampleRate=std::max(8000.0,sr); channels={}; phase=0; rng=0x428124ab;
    smoothing=1-std::exp(-1.0/(.025*sampleRate));
    low=420; high=2800; drive=3.2f; saturation=1; flutter=.045f; rate=5.1f; hiss=.01f; mix=1; output=1;
  }
  void process(float* const* data,int count,int samples,int preset,float tone,float grit,float noise,float wet,float gainDb,float bandwidth=0,bool am=true,bool fm=true,bool sw=true) {
    const auto& p=presets[std::clamp(preset,0,9)];
    const float toneScale=std::pow(2.f,std::clamp(tone,-1.f,1.f));
    const float targetLow=std::clamp(p.low*toneScale*std::pow(2.f,-bandwidth),30.f,float(sampleRate*.2));
    const float targetHigh=std::clamp(p.high*toneScale*std::pow(2.f,bandwidth),targetLow+50,float(sampleRate*.45));
    const float targetDrive=1+(p.drive-1)*std::clamp(grit,0.f,2.f)*(am?1.f:0.f);
    const float targetOutput=std::pow(10.f,std::clamp(gainDb,-24.f,6.f)/20);
    for(int i=0;i<samples;++i) {
      saturation=smooth(saturation,am?1.f:0.f);
      low=smooth(low,targetLow); high=smooth(high,targetHigh); drive=smooth(drive,targetDrive);
      flutter=smooth(flutter,fm?p.flutter:0.f); rate=smooth(rate,p.rate); hiss=smooth(hiss,p.noise*std::clamp(noise,0.f,2.f)*(sw?1.f:0.f));
      mix=smooth(mix,std::clamp(wet,0.f,1.f)); output=smooth(output,targetOutput);
      phase+=6.283185307179586*rate/sampleRate; if(phase>6.283185307179586) phase-=6.283185307179586;
      const float wobble=std::sin(phase);
      const float a=1-std::exp(float(-6.283185307179586*low/sampleRate));
      const float b=1-std::exp(float(-6.283185307179586*high*(1+flutter*wobble)/sampleRate));
      for(int c=0;c<std::min(count,2);++c) {
        auto& s=channels[c]; const float dry=std::isfinite(data[c][i])?data[c][i]:0;
        const float x=std::clamp(dry,-8.f,8.f);
        s.envelope+=(std::abs(x)-s.envelope)*(std::abs(x)>s.envelope?.01f:.0002f);
        s.hp1+=a*(x-s.hp1); float y=x-s.hp1;
        s.hp2+=a*(y-s.hp2); y-=s.hp2;
        y+=saturation*(std::tanh(y*drive)/std::sqrt(drive)-y);
        rng^=rng<<13; rng^=rng>>17; rng^=rng<<5;
        y+=(float(rng)/4294967295.f*2-1)*hiss*std::min(s.envelope*10,1.f);
        s.lp1+=b*(y-s.lp1); s.lp2+=b*(s.lp1-s.lp2);
        y=s.lp2*(1-.3f*flutter+.3f*flutter*wobble);
        data[c][i]=(dry*(1-mix)+y*mix)*output;
      }
    }
  }
};
}
