#pragma once

#include "../component/Rigidbody.hpp"
#include "../component/Transform.hpp"

namespace mir::movement {
    inline void Update(const Id id, const float deltaTime) {
        float velocityX = rigidbody::VelocityX::Get(id);
        float velocityY = rigidbody::VelocityY::Get(id);

        velocityY += rigidbody::OnGround::Get(id) ? 0 : rigidbody::Gravity::Get(id) * deltaTime;

        float currentPosX = transform::PositionX::Get(id);
        float currentPosY = transform::PositionY::Get(id);

        transform::SetPosition(id, currentPosX + velocityX * deltaTime, currentPosY + velocityY * deltaTime);
    }
}