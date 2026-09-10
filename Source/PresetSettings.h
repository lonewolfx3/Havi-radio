#pragma once
#include <array>
namespace havi {
struct PresetSettings {
  float bandwidth, tone, grit, noise, mix, output;
  bool am, fm, sw;
};
inline constexpr std::array<PresetSettings, 10> settings {{
  {-.20f,-.15f,1.15f,1.00f,1.00f,-1.0f,true,true,true},
  { .15f, .10f, .80f, .55f, .90f,-.5f,true,true,true},
  { .20f, .05f, .90f, .80f, .95f,-.8f,true,true,true},
  { .35f, .20f, .65f, .35f, .82f,0.f,true,true,true},
  {-.10f, .15f,1.35f, .65f, .95f,-2.f,true,true,true},
  {-.50f,-.65f, .55f, .40f,1.00f,1.f,true,true,true},
  { .55f, .25f, .30f,0.f, .75f,0.f,true,false,false},
  {-.15f,-.05f,1.25f, .90f, .92f,-1.5f,true,true,true},
  {-.60f, .35f, .75f, .30f,1.00f,-.5f,true,false,true},
  {-.40f,-.35f,1.50f,1.20f,1.00f,-2.5f,true,true,true}
}};
}
