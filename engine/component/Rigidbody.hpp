#pragma once

#include "../core/Component.hpp"

namespace mir::rigidbody {
    struct VelocityX : public Component<VelocityX, float> {};
    struct VelocityY : public Component<VelocityY, float> {};
    struct Gravity : public Component<Gravity, float> {};
    struct OnGround : public Component<OnGround, bool> {};

    inline void SetVelocity(const Id id, const float x, const float y) noexcept {
        VelocityX::Set(id, x);
        VelocityY::Set(id, y);
    }
}