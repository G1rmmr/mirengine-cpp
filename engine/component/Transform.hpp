#pragma once

#include "../core/Component.hpp"

namespace mir::transform {
    struct PositionX : public Component<PositionX, float> {};
    struct PositionY : public Component<PositionY, float> {};
    struct Rotation : public Component<Rotation, float> {};
    struct Scale : public Component<Scale, float> {};

    // Local transforms remain the authoring-facing components. World transforms
    // are derived by hierarchy::UpdateWorldTransforms after simulation commits.
    struct WorldPositionX : public Component<WorldPositionX, float> {};
    struct WorldPositionY : public Component<WorldPositionY, float> {};
    struct WorldRotation : public Component<WorldRotation, float> {};
    struct WorldScale : public Component<WorldScale, float> {};

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

        struct WorldTransformPayload {
            Id EntityId;
            float X;
            float Y;
            float Rotation;
            float Scale;
        };

        inline void ApplyWorldTransform(const void* rawPayload) noexcept {
            const auto& payload = *static_cast<const WorldTransformPayload*>(rawPayload);
            WorldPositionX::ApplyCommitted(payload.EntityId, payload.X);
            WorldPositionY::ApplyCommitted(payload.EntityId, payload.Y);
            WorldRotation::ApplyCommitted(payload.EntityId, payload.Rotation);
            WorldScale::ApplyCommitted(payload.EntityId, payload.Scale);
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

	[[nodiscard]] inline bool SetRotation(const Id id, const float rotation) noexcept {
		return Rotation::Set(id, rotation);
	}

	[[nodiscard]] inline bool SetScale(const Id id, const float scale) noexcept {
		return Scale::Set(id, scale);
	}

    [[nodiscard]] inline bool RegisterWorldTransformStorage() noexcept {
        auto& manager = core::Manager::Instance();
        return manager.RegisterCleanup(&WorldPositionX::Remove) &&
            manager.RegisterCleanup(&WorldPositionY::Remove) &&
            manager.RegisterCleanup(&WorldRotation::Remove) &&
            manager.RegisterCleanup(&WorldScale::Remove);
    }

    // Reserved for derived-transform systems. Keeping it command-buffered
    // preserves the same structural-update rules as ordinary components.
    [[nodiscard]] inline bool SetWorldTransform(
        const Id id,
        const float x,
        const float y,
        const float rotation,
        const float scale) noexcept {
        auto& manager = core::Manager::Instance();
        if (!manager.IsValidEntity(id) || !RegisterWorldTransformStorage()) return false;
        return manager.Enqueue<detail::WorldTransformPayload>(
            &detail::ApplyWorldTransform,
            detail::WorldTransformPayload{id, x, y, rotation, scale});
    }
}
