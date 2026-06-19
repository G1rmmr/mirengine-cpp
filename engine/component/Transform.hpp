#pragma once

#include "../core/Component.hpp"

namespace mir::transform {
    struct PositionX : public Component<PositionX, float> {};
    struct PositionY : public Component<PositionY, float> {};
    struct Rotation : public Component<Rotation, float> {};
    struct Scale : public Component<Scale, float> {};

    inline void SetPosition(const Id id, const float x, const float y) noexcept {
    	PositionX::Set(id, x);
    	PositionY::Set(id, y);
	}
}
