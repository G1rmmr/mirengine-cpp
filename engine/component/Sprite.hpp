#pragma once

#include "../core/Component.hpp"
#include <String.hpp>

namespace mir::sprite {
    struct Texture : public Component<Texture, String<>> {};
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