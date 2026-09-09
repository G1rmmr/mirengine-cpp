#pragma once

#include "../core/Component.hpp"

namespace mir::transform {
    struct PositionX : public Component<PositionX, float> {};
    struct PositionY : public Component<PositionY, float> {};
    struct Rotation : public Component<Rotation, float> {};
    struct Scale : public Component<Scale, float> {};

    namespace detail {
        struct PositionPayload {
            Id EntityId;
            float X;
            float Y;
        };

        inline void ApplyPosition(const void* rawPayload) noexcept {
            const auto& payload = *static_cast<const PositionPayload*>(rawPayload);
            PositionX::ApplyCommitted(payload.EntityId, payload.X);
            PositionY::ApplyCommitted(payload.EntityId, payload.Y);
        }
    }

    // Position is one logical state transition, so reserve one command-buffer
    // entry and apply both coordinates together.
    inline bool SetPosition(const Id id, const float x, const float y) noexcept {
        auto& manager = core::Manager::Instance();
        if (!manager.IsValidEntity(id) ||
            !manager.RegisterCleanup(&PositionX::Remove) ||
            !manager.RegisterCleanup(&PositionY::Remove)) {
            return false;
        }
        return manager.Enqueue<detail::PositionPayload>(
            &detail::ApplyPosition, detail::PositionPayload{id, x, y});
    }
}
