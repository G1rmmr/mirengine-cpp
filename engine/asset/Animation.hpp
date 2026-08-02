#pragma once

#include <container/String.hpp>
#include <container/List.hpp>
#include "../core/Entity.hpp"

using namespace zet;

namespace mir::animation {
	inline constexpr std::size_t MAX_ANIMATION = 256;
	inline constexpr std::size_t MAX_ANIMATION_FRAMES = 256;

	struct Frame {
        float X = 0.f;
        float Y = 0.f;
        float Width = 0.f;
        float Height = 0.f;
	};
	using Frames = List<Frame, MAX_ANIMATION_FRAMES>;

	void Register(const String<>& name, const Frames& frames) noexcept;

    void Play(const Id id, const String<>& animName, float speed = 1.f, bool loop = true) noexcept;
    void Stop(const Id id) noexcept;
}
