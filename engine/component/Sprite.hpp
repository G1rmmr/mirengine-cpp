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

		struct PairPayload {
			Id EntityId;
			float First;
			float Second;
		};

		struct TintPayload {
			Id EntityId;
			std::uint8_t Red;
			std::uint8_t Green;
			std::uint8_t Blue;
		};

		inline void ApplySourceSize(const void* rawPayload) noexcept {
			const auto& payload = *static_cast<const PairPayload*>(rawPayload);
			SourceWidth::ApplyCommitted(payload.EntityId, payload.First);
			SourceHeight::ApplyCommitted(payload.EntityId, payload.Second);
		}

		inline void ApplyDestinationSize(const void* rawPayload) noexcept {
			const auto& payload = *static_cast<const PairPayload*>(rawPayload);
			DestinationWidth::ApplyCommitted(payload.EntityId, payload.First);
			DestinationHeight::ApplyCommitted(payload.EntityId, payload.Second);
		}

		inline void ApplyAnchor(const void* rawPayload) noexcept {
			const auto& payload = *static_cast<const PairPayload*>(rawPayload);
			AnchorX::ApplyCommitted(payload.EntityId, payload.First);
			AnchorY::ApplyCommitted(payload.EntityId, payload.Second);
		}

		inline void ApplyTint(const void* rawPayload) noexcept {
			const auto& payload = *static_cast<const TintPayload*>(rawPayload);
			TintRed::ApplyCommitted(payload.EntityId, payload.Red);
			TintGreen::ApplyCommitted(payload.EntityId, payload.Green);
			TintBlue::ApplyCommitted(payload.EntityId, payload.Blue);
		}
    }

	[[nodiscard]] inline bool SetTexture(const Id id, const String<>& texture) noexcept {
		return Texture::Set(id, texture);
	}

	[[nodiscard]] inline bool SetSourceSize(const Id id, const float width, const float height) noexcept {
		auto& manager = core::Manager::Instance();
		if (!manager.IsValidEntity(id) || !manager.RegisterCleanup(&SourceWidth::Remove) ||
			!manager.RegisterCleanup(&SourceHeight::Remove)) return false;
		return manager.Enqueue<detail::PairPayload>(&detail::ApplySourceSize, detail::PairPayload{id, width, height});
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

	[[nodiscard]] inline bool SetDestinationSize(const Id id, const float width, const float height) noexcept {
		auto& manager = core::Manager::Instance();
		if (!manager.IsValidEntity(id) || !manager.RegisterCleanup(&DestinationWidth::Remove) ||
			!manager.RegisterCleanup(&DestinationHeight::Remove)) return false;
		return manager.Enqueue<detail::PairPayload>(&detail::ApplyDestinationSize, detail::PairPayload{id, width, height});
    }

	[[nodiscard]] inline bool SetAnchor(const Id id, const float x, const float y) noexcept {
		auto& manager = core::Manager::Instance();
		if (!manager.IsValidEntity(id) || !manager.RegisterCleanup(&AnchorX::Remove) ||
			!manager.RegisterCleanup(&AnchorY::Remove)) return false;
		return manager.Enqueue<detail::PairPayload>(&detail::ApplyAnchor, detail::PairPayload{id, x, y});
    }

	[[nodiscard]] inline bool SetTint(
        const Id id, 
        const std::uint8_t red, 
        const std::uint8_t green, 
        const std::uint8_t blue) noexcept {
		auto& manager = core::Manager::Instance();
		if (!manager.IsValidEntity(id) || !manager.RegisterCleanup(&TintRed::Remove) ||
			!manager.RegisterCleanup(&TintGreen::Remove) || !manager.RegisterCleanup(&TintBlue::Remove)) return false;
		return manager.Enqueue<detail::TintPayload>(&detail::ApplyTint, detail::TintPayload{id, red, green, blue});
    }

	[[nodiscard]] inline bool SetZindex(const Id id, const std::uint16_t zindex) noexcept {
		return Zindex::Set(id, zindex);
	}

	[[nodiscard]] inline bool SetAlpha(const Id id, const std::uint8_t alpha) noexcept {
		return Alpha::Set(id, alpha);
	}
}
