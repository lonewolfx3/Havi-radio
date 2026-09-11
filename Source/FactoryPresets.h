#pragma once
#include <array>
#include <algorithm>
#include <cmath>
namespace havi {
inline constexpr std::array<const char*,16> controlIDs{"loRoll","midEq","hiRoll","bandwidth","tuning","filter","drive","tone","width","mix","output","am","fm","sw","dropout","power"};
using Controls=std::array<float,16>;
inline constexpr std::array<Controls,10> factoryControls{{
 {{360,-2.5f,3300,38,-32,42,68,-.28f,36,92,-1.5f,1,0,0,1,1}},
 {{180,1.f,7200,72,18,70,34,.20f,78,76,-.5f,0,1,0,0,1}},
 {{220,2.5f,6200,64,-8,58,48,.08f,92,84,-1.f,0,1,0,0,1}},
 {{110,3.f,9800,88,42,82,22,.45f,100,62,.5f,0,1,0,0,1}},
 {{290,-1.f,4800,52,5,50,78,-.12f,62,100,-3.f,1,0,0,1,1}},
 {{70,-5.f,1100,24,-55,18,20,-.72f,48,100,1.f,0,0,1,0,1}},
 {{45,.5f,14500,96,0,94,16,.35f,100,68,0.f,0,1,0,0,1}},
 {{170,4.f,5600,60,27,62,84,-.18f,74,90,-2.f,0,1,0,1,1}},
 {{620,-3.5f,2600,30,-18,34,56,.22f,18,100,-1.f,1,0,0,0,1}},
 {{520,-4.f,2100,20,-68,26,88,-.42f,28,86,-3.5f,1,0,0,1,1}}
}};
inline bool matchesFactory(const Controls& c,int index){
 const auto& e=factoryControls[size_t(std::clamp(index,0,9))];
 for(size_t i=0;i<c.size();++i)if(std::abs(c[i]-e[i])>.005f)return false;
 return true;
}
inline float visualStep(float current,float target,float elapsed){const auto next=current+(std::clamp(target,0.f,1.f)-current)*(1-std::exp(-std::max(elapsed,0.f)/.055f));return std::abs(next-target)<.0001f?target:next;}
}
