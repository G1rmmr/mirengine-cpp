#pragma once

#include "../core/Component.hpp"
#include <container/String.hpp>

namespace mir::sprite {
    struct Texture : public Component<Texture, String<>> {};
    struct SourceX : public Component<SourceX, float> {};
    struct SourceY : public Component<SourceY, float> {};
    struct SourceWidth : public Component<SourceWidth, float> {};
    struct SourceHeight : public Component<SourceHeight, float> {};
    struct DestinationWidth : public Component<DestinationWidth, float> {};
    struct DestinationHeight : public Component<DestinationHeight, float> {};
    struct AnchorX : public Component<AnchorX, float> {};
    struct AnchorY : public Component<AnchorY, float> {};
    struct Zindex : public Component<Zindex, std::uint16_t> {};
    struct Alpha : public Component<Alpha, std::uint8_t> {};
    struct TintRed : public Component<TintRed, std::uint8_t> {};
    struct TintGreen : public Component<TintGreen, std::uint8_t> {};
    struct TintBlue : public Component<TintBlue, std::uint8_t> {};

	inline void SetSourceSize(const Id id, const float width, const float height) noexcept {
        SourceWidth::Set(id, width);
        SourceHeight::Set(id, height);
    }

    namespace detail {
        struct SourceRectPayload {
            Id EntityId;
            float X;
            float Y;
            float Width;
            float Height;
        };

        inline void ApplySourceRect(const void* rawPayload) noexcept {
            const auto& payload = *static_cast<const SourceRectPayload*>(rawPayload);
            SourceX::ApplyCommitted(payload.EntityId, payload.X);
            SourceY::ApplyCommitted(payload.EntityId, payload.Y);
            SourceWidth::ApplyCommitted(payload.EntityId, payload.Width);
            SourceHeight::ApplyCommitted(payload.EntityId, payload.Height);
        }
    }

    // The source rectangle is intentionally stored as ECS data rather than as
    // renderer state, so sprites remain serializable and backend-independent.
    // It is applied as one command to avoid partially changed sprite frames.
    inline bool SetSourceRect(
        const Id id,
        const float x,
        const float y,
        const float width,
        const float height) noexcept {
        auto& manager = core::Manager::Instance();
        if (!manager.IsValidEntity(id) ||
            !manager.RegisterCleanup(&SourceX::Remove) ||
            !manager.RegisterCleanup(&SourceY::Remove) ||
            !manager.RegisterCleanup(&SourceWidth::Remove) ||
            !manager.RegisterCleanup(&SourceHeight::Remove)) {
            return false;
        }
        return manager.Enqueue<detail::SourceRectPayload>(
            &detail::ApplySourceRect, detail::SourceRectPayload{id, x, y, width, height});
    }

	inline void SetDestinationSize(const Id id, const float width, const float height) noexcept {
        DestinationWidth::Set(id, width);
        DestinationHeight::Set(id, height);
    }

	inline void SetAnchor(const Id id, const float x, const float y) noexcept {
        AnchorX::Set(id, x);
        AnchorY::Set(id, y);
    }

	inline void SetTint(
        const Id id, 
        const std::uint8_t red, 
        const std::uint8_t green, 
        const std::uint8_t blue) noexcept {
        TintRed::Set(id, red);
        TintGreen::Set(id, green);
        TintBlue::Set(id, blue);
    }
}
