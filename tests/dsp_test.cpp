#ifdef NDEBUG
#undef NDEBUG
#endif
#include "../Source/RadioEngine.h"
#include "../Source/Meter.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
int main(){havi::Meter meter;meter.push(-.5f);meter.push(.25f);assert(meter.take()==.5f);float position=0;for(int i=0;i<60;++i)position=havi::Meter::animate(position,1,1.f/60);assert(position>.99f);for(int i=0;i<180;++i)position=havi::Meter::animate(position,0,1.f/60);assert(position<.0001f);
 for(double sr:{8000.,44100.,48000.,96000.,192000.}){havi::RadioEngine e;e.prepare(sr);std::vector<float> l(2048),r(2048);float* data[]={l.data(),r.data()};for(int preset=0;preset<10;++preset){double energy=0;for(int block=0;block<8;++block){for(int i=0;i<2048;++i){l[i]=.2f*std::sin(float((block*2048+i)*6.28318530718*1000/sr));r[i]=0;}e.process(data,2,2048,200,-2,6000,60,0,65,55,0,100,85,-1,(preset&1)!=0,(preset&2)!=0,(preset&4)!=0,preset%2,true);for(float x:l){assert(std::isfinite(x));assert(std::abs(x)<4);energy+=x*x;}}assert(energy>.000001);}e.prepare(sr);std::fill(l.begin(),l.end(),.125f);e.process(data,1,2048,200,0,6000,50,0,50,50,0,100,100,0,true,true,true,false,false);assert(l.back()==.125f);}
 {havi::RadioEngine withDropout,clean;withDropout.prepare(48000);clean.prepare(48000);std::vector<float>a(1024),b(1024);float*pa[]={a.data()};float*pb[]={b.data()};for(int i=0;i<1024;++i)a[i]=b[i]=.2f*std::sin(float(i)*.17f);withDropout.process(pa,1,1024,200,0,6000,50,0,50,50,0,100,100,0,true,true,true,true,true);clean.process(pb,1,1024,200,0,6000,50,0,50,50,0,100,100,0,true,true,true,false,true);for(int i=0;i<1024;++i)a[i]=b[i]=.15f*std::cos(float(i)*.11f);withDropout.process(pa,1,1024,200,0,6000,50,0,50,50,0,100,100,0,true,true,true,false,true);clean.process(pb,1,1024,200,0,6000,50,0,50,50,0,100,100,0,true,true,true,false,true);for(int i=0;i<1024;++i)assert(std::abs(a[i]-b[i])<1.0e-6f);}
 std::cout<<"PASS: RadioBox DSP, metering, dropout, width and power bypass.\n";}
