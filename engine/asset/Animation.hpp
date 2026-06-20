#pragma once

#include <container/String.hpp>
#include <container/List.hpp>

using namespace zet;

namespace mir::animation {
    struct Frame {
        float X = 0.f;
        float Y = 0.f;
        float Width = 0.f;
        float Height = 0.f;
    };

    void Register(const String<>& name, const List<Frame>& frames) noexcept;

    void Play(const Id id, const String<>& animName, float speed = 1.f, bool loop = true) noexcept;
    void Stop(const Id id) noexcept;
}