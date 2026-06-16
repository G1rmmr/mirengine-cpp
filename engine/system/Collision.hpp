#pragma once

#include "../core/Entity.hpp"
#include "../component/Collider.hpp"
#include "../component/Transform.hpp"

namespace mir::collision {
    inline bool Update(const Id lhs, const Id rhs) {
        if (!collider::ShouldTrigger::Get(lhs) ||
            !collider::ShouldTrigger::Get(rhs)){
            return false;
        }

        float lhsLeft = transform::PositionX::Get(lhs) + collider::OffsetX::Get(lhs);
        float lhsTop = transform::PositionY::Get(lhs) + collider::OffsetY::Get(lhs);
        float lhsRight = lhsLeft + collider::BoundX::Get(lhs);
        float lhsBottom = lhsTop + collider::BoundY::Get(lhs);

        float rhsLeft = transform::PositionX::Get(rhs) + collider::OffsetX::Get(rhs);
        float rhsTop = transform::PositionY::Get(rhs) + collider::OffsetY::Get(rhs);
        float rhsRight = rhsLeft + collider::BoundX::Get(rhs);
        float rhsBottom = rhsTop + collider::BoundY::Get(rhs);

        return  lhsRight > rhsLeft && lhsBottom > rhsTop && rhsRight > lhsLeft && rhsBottom > lhsTop;
    }
}