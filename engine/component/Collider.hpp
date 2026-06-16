#pragma once

#include "../core/Component.hpp"

namespace mir::collider {
    struct BoundX : public Component<BoundX, float> {};
    struct BoundY : public Component<BoundY, float> {};
    struct OffsetX : public Component<OffsetX, float> {};
    struct OffsetY : public Component<OffsetY, float> {};
    struct ShouldTrigger : public Component<ShouldTrigger, bool> {};

    inline void SetBound(const Id id, const float x, const float y) noexcept {
        BoundX::Set(id, x);
        BoundY::Set(id, y);
    }

    inline void SetOffset(const Id id, const float x, const float y) noexcept {
        OffsetX::Set(id, x);
        OffsetY::Set(id, y);
    }
}