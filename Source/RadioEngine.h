#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
namespace havi {
inline constexpr std::array<const char*,10> presetNames{{"VINTAGE RADIO","RADIO 2000'S","80'S RADIO","SUMMER 2015","URBAN CITY RADIO","UNDER WATER RADIO","MODERN RADIO","BOOM BOX RADIO","TELEPHONE RADIO","MIDNIGHT AM"}};
class RadioEngine {
 struct Channel{float hp=0,lp=0,midLp=0;};std::array<Channel,2> state{};double sampleRate=48000,phase=0;uint32_t rng=0x428124ab;float dropoutGain=1;
 float randomUnit(){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return float(rng)/4294967295.f;}
public:
 void prepare(double sr){sampleRate=std::max(8000.0,sr);state={};phase=0;rng=0x428124ab;dropoutGain=1;}
 void process(float* const* data,int channels,int samples,float lo,float midDb,float hi,float bandwidth,float tuning,float filter,float drive,float tone,float width,float wet,float outDb,bool am,bool fm,bool sw,bool dropout,bool power){
  if(!power)return;
  const float tune=std::pow(2.f,std::clamp(tuning,-80.f,100.f)/240.f),bandWidth=.35f+std::clamp(bandwidth,0.f,100.f)*.012f;
  float low=std::clamp(lo*tune/std::max(.5f,bandWidth),20.f,float(sampleRate*.22));float high=std::clamp(hi*tune*bandWidth,low+80,float(sampleRate*.47));
  high*=.35f+.65f*std::clamp(filter,0.f,100.f)/100.f;high*=std::pow(2.f,std::clamp(tone,-1.f,1.f));
  const int enabledBands=int(am)+int(fm)+int(sw);
  if(enabledBands>0){
   const float divisor=1.f/float(enabledBands);
   low*=(am*1.35f+fm*1.f+sw*1.f)*divisor;
   high*=(am*.62f+fm*1.f+sw*.78f)*divisor;
  }
  const float hpA=1-std::exp(float(-6.28318530718*low/sampleRate)),lpA=1-std::exp(float(-6.28318530718*high/sampleRate)),midA=1-std::exp(float(-6.28318530718*1100/sampleRate));
  const float midGain=std::pow(10.f,std::clamp(midDb,-12.f,12.f)/20.f)-1,gain=std::pow(10.f,std::clamp(outDb,-24.f,6.f)/20.f),saturation=1+std::clamp(drive,0.f,100.f)*.055f,mix=std::clamp(wet,0.f,100.f)/100.f,sideGain=std::clamp(width,0.f,200.f)/100.f;
  if(!dropout)dropoutGain=1.f;
  const float flutterHz=enabledBands>0?(am*2.3f+fm*5.2f+sw*8.5f)/float(enabledBands):5.2f;
  for(int i=0;i<samples;++i){if(dropout){phase+=6.28318530718*flutterHz/sampleRate;if(phase>6.28318530718)phase-=6.28318530718;const float targetDrop=randomUnit()<.00022f?(.10f+.22f*randomUnit()):1.f;dropoutGain+=(targetDrop-dropoutGain)*(targetDrop<dropoutGain?.15f:.0025f);}float dry[2]{0,0},fx[2]{0,0};
   for(int c=0;c<std::min(channels,2);++c){dry[c]=std::isfinite(data[c][i])?std::clamp(data[c][i],-8.f,8.f):0.f;auto& s=state[size_t(c)];s.hp+=hpA*(dry[c]-s.hp);float y=dry[c]-s.hp;s.midLp+=midA*(y-s.midLp);y+=s.midLp*midGain;y=std::tanh(y*saturation)/std::sqrt(saturation);s.lp+=lpA*(y-s.lp);const float dropoutOnly=dropout?dropoutGain*(1-.025f*std::sin(phase)):1.f;const float dropoutNoise=dropout?(randomUnit()*2-1)*.012f:0.f;fx[c]=(s.lp+dropoutNoise)*dropoutOnly;}
   if(channels>=2){const float m=(fx[0]+fx[1])*.5f,s=(fx[0]-fx[1])*.5f*sideGain;fx[0]=m+s;fx[1]=m-s;}for(int c=0;c<std::min(channels,2);++c)data[c][i]=(dry[c]*(1-mix)+fx[c]*mix)*gain;
  }
 }
};}
