#pragma once

#include "../component/Rigidbody.hpp"
#include "../component/Transform.hpp"
#include "../math/Simd.hpp"

namespace mir::movement {
    inline void Update(const Id id, const float deltaTime) {
        float velocityX = rigidbody::VelocityX::Get(id);
        float velocityY = rigidbody::VelocityY::Get(id);

        velocityY += rigidbody::OnGround::Get(id) ? 0 : rigidbody::Gravity::Get(id) * deltaTime;

        // Pack position and velocity into 128-bit SIMD registers
        simd::Floats pos = simd::Set(transform::PositionX::Get(id), transform::PositionY::Get(id), 0.f, 0.f);
        simd::Floats vel = simd::Set(velocityX, velocityY, 0.f, 0.f);
        simd::Floats dt = simd::Set(deltaTime);

        // Perform parallel position update: pos + vel * dt
        simd::Floats newPos = simd::Add(pos, simd::Mul(vel, dt));

        // Extract updated components back
        alignas(16) float temp[4];
        simd::Store(temp, newPos);

        transform::SetPosition(id, temp[0], temp[1]);
    }
}