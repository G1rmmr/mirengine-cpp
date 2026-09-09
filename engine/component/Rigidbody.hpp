#pragma once

#include "../core/Component.hpp"

namespace mir::rigidbody {
    struct VelocityX : public Component<VelocityX, float> {};
    struct VelocityY : public Component<VelocityY, float> {};
    struct Gravity : public Component<Gravity, float> {};
    struct OnGround : public Component<OnGround, bool> {};

    namespace detail {
        struct VelocityPayload {
            Id EntityId;
            float X;
            float Y;
        };

        inline void ApplyVelocity(const void* rawPayload) noexcept {
            const auto& payload = *static_cast<const VelocityPayload*>(rawPayload);
            VelocityX::ApplyCommitted(payload.EntityId, payload.X);
            VelocityY::ApplyCommitted(payload.EntityId, payload.Y);
        }
    }

    inline bool SetVelocity(const Id id, const float x, const float y) noexcept {
        auto& manager = core::Manager::Instance();
        if (!manager.IsValidEntity(id) ||
            !manager.RegisterCleanup(&VelocityX::Remove) ||
            !manager.RegisterCleanup(&VelocityY::Remove)) {
            return false;
        }
        return manager.Enqueue<detail::VelocityPayload>(
            &detail::ApplyVelocity, detail::VelocityPayload{id, x, y});
    }
}
