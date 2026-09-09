#pragma once

#include "../core/Entity.hpp"
#include "../math/Math.hpp"

using namespace zet;

namespace mir::camera {
    inline float CurrentZoom = 1.f;
    inline float ShakePower = 0.f;
    inline float ShakeTime = 0.f;

    inline Id Target = 0;

    inline void Follow(const Id target) { Target = target; }
    inline void SetZoom(const float zoom) { CurrentZoom = zoom; }

    inline void SetPosition(float x, float y) {
        TargetID = 0;
    }

    inline void Shake(float intensity, float duration) {
        ShakePower = intensity;
        ShakeTime = duration;
    }
}
