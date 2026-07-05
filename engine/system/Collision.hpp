#pragma once

#include "../core/Entity.hpp"
#include "../component/Collider.hpp"
#include "../component/Transform.hpp"

namespace mir::collision {
    inline bool Update(const Id lhs, const Id rhs) {
        if (!collider::BoundX::IsValidEntity(lhs) || !collider::BoundY::IsValidEntity(lhs) ||
            !collider::BoundX::IsValidEntity(rhs) || !collider::BoundY::IsValidEntity(rhs) ||
            !transform::PositionX::IsValidEntity(lhs) || !transform::PositionY::IsValidEntity(lhs) ||
            !transform::PositionX::IsValidEntity(rhs) || !transform::PositionY::IsValidEntity(rhs)) {
            return false;
        }

        bool lhsTrigger = collider::ShouldTrigger::IsValidEntity(lhs) ? collider::ShouldTrigger::Get(lhs) : false;
        bool rhsTrigger = collider::ShouldTrigger::IsValidEntity(rhs) ? collider::ShouldTrigger::Get(rhs) : false;
        if (!lhsTrigger || !rhsTrigger) {
            return false;
        }

        float lhsOffsetX = collider::OffsetX::IsValidEntity(lhs) ? collider::OffsetX::Get(lhs) : 0.f;
        float lhsOffsetY = collider::OffsetY::IsValidEntity(lhs) ? collider::OffsetY::Get(lhs) : 0.f;
        float rhsOffsetX = collider::OffsetX::IsValidEntity(rhs) ? collider::OffsetX::Get(rhs) : 0.f;
        float rhsOffsetY = collider::OffsetY::IsValidEntity(rhs) ? collider::OffsetY::Get(rhs) : 0.f;

        float lhsLeft = transform::PositionX::Get(lhs) + lhsOffsetX;
        float lhsTop = transform::PositionY::Get(lhs) + lhsOffsetY;
        float lhsRight = lhsLeft + collider::BoundX::Get(lhs);
        float lhsBottom = lhsTop + collider::BoundY::Get(lhs);

        float rhsLeft = transform::PositionX::Get(rhs) + rhsOffsetX;
        float rhsTop = transform::PositionY::Get(rhs) + rhsOffsetY;
        float rhsRight = rhsLeft + collider::BoundX::Get(rhs);
        float rhsBottom = rhsTop + collider::BoundY::Get(rhs);

        return  lhsRight > rhsLeft && lhsBottom > rhsTop && rhsRight > lhsLeft && rhsBottom > lhsTop;
    }
}
