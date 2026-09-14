#pragma once

#include "../core/Component.hpp"

namespace mir::collider {
    struct BoundX : public Component<BoundX, float> {};
    struct BoundY : public Component<BoundY, float> {};
    struct OffsetX : public Component<OffsetX, float> {};
    struct OffsetY : public Component<OffsetY, float> {};
    struct ShouldTrigger : public Component<ShouldTrigger, bool> {};

    namespace detail {
        struct PairPayload {
            Id EntityId;
            float X;
            float Y;
        };

        inline void ApplyBound(const void* rawPayload) noexcept {
            const auto& payload = *static_cast<const PairPayload*>(rawPayload);
            BoundX::ApplyCommitted(payload.EntityId, payload.X);
            BoundY::ApplyCommitted(payload.EntityId, payload.Y);
        }

        inline void ApplyOffset(const void* rawPayload) noexcept {
            const auto& payload = *static_cast<const PairPayload*>(rawPayload);
            OffsetX::ApplyCommitted(payload.EntityId, payload.X);
            OffsetY::ApplyCommitted(payload.EntityId, payload.Y);
        }
    }

    inline bool SetBound(const Id id, const float x, const float y) noexcept {
        auto& manager = core::Manager::Instance();
        if (!manager.IsValidEntity(id) ||
            !manager.RegisterCleanup(&BoundX::Remove) ||
            !manager.RegisterCleanup(&BoundY::Remove)) {
            return false;
        }
        return manager.Enqueue<detail::PairPayload>(
            &detail::ApplyBound, detail::PairPayload{id, x, y});
    }

    inline bool SetOffset(const Id id, const float x, const float y) noexcept {
        auto& manager = core::Manager::Instance();
        if (!manager.IsValidEntity(id) ||
            !manager.RegisterCleanup(&OffsetX::Remove) ||
            !manager.RegisterCleanup(&OffsetY::Remove)) {
            return false;
        }
        return manager.Enqueue<detail::PairPayload>(
            &detail::ApplyOffset, detail::PairPayload{id, x, y});
    }

	[[nodiscard]] inline bool SetShouldTrigger(const Id id, const bool shouldTrigger) noexcept {
		return ShouldTrigger::Set(id, shouldTrigger);
	}
}
