#pragma once
#include <atomic>
#include <algorithm>
#include <cmath>
namespace havi {
// Audio thread publishes peaks; UI consumes them without a lock at 60 Hz.
// This is a dBFS input peak indicator with analog-like movement, not calibrated VU.
class Meter {
  std::atomic<float> peak{0};
public:
  void reset() { peak.store(0,std::memory_order_relaxed); }
  void push(float sample) {
    if(!std::isfinite(sample)) return;
    const float value=std::abs(sample);
    float previous=peak.load(std::memory_order_relaxed);
    while(value>previous&&!peak.compare_exchange_weak(previous,value,std::memory_order_relaxed)) {}
  }
  float take() { return peak.exchange(0,std::memory_order_relaxed); }
  static float animate(float displayed,float input,float elapsed) {
    const float target=std::clamp((20*std::log10(std::max(input,0.000001f))+48)/48,0.f,1.f);
    const float coefficient=1-std::exp(-elapsed/(target>displayed?.035f:.25f));
    return displayed+coefficient*(target-displayed);
  }
};
}
