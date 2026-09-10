#ifdef NDEBUG
#undef NDEBUG
#endif
#include "../Source/RadioEngine.h"
#include <cassert>
#include "../Source/Meter.h"
#include <iostream>
#include <vector>
int main() {
  havi::Meter meter;
  meter.push(-.5f); meter.push(.25f);
  assert(meter.take()==.5f); assert(meter.take()==0);
  float position=0;
  for(int i=0;i<60;++i) position=havi::Meter::animate(position,1,1.f/60);
  assert(position>.99f);
  for(int i=0;i<180;++i) position=havi::Meter::animate(position,0,1.f/60);
  assert(position<.0001f);

  for(double sr:{8000.,44100.,48000.,96000.,192000.}) {
    havi::RadioEngine e; e.prepare(sr);
    std::vector<float> left(4096),right(4096); float* data[]={left.data(),right.data()};
    for(int p=0;p<10;++p) {
      double energy=0;
      for(int block=0;block<20;++block) {
        for(int i=0;i<4096;++i) {left[i]=.2f*std::sin(float((block*4096+i)*6.28318530718*1000/sr));right[i]=0;}
        e.process(data,2,4096,p,p%2?1.f:-1.f,2,2,1,6,p%2?1.f:-1.f,p%2==0,p%3==0,p%4==0);
        for(int i=0;i<4096;++i) {assert(std::isfinite(left[i]));assert(std::abs(left[i])<4);assert(right[i]==0);energy+=left[i]*left[i];}
      }
      assert(energy>0.000001);
    }
    e.prepare(sr);
    for(int b=0;b<30;++b) {std::fill(left.begin(),left.end(),.125f);e.process(data,1,4096,0,0,1,0,0,0);}
    assert(std::abs(left.back()-.125f)<.00001f);
    e.prepare(sr);std::fill(left.begin(),left.end(),0);std::fill(right.begin(),right.end(),0);
    e.process(data,2,4096,9,0,2,2,1,0);
    for(auto x:left) assert(x==0);
  }
  std::cout<<"PASS: all 10 presets at 5 sample rates; finite bounded output, channel isolation, silent input, dry mix.\n";
}
