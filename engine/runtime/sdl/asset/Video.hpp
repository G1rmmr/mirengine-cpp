#pragma once

#include <container/String.hpp>

using namespace zet;

namespace mir::video {
    void Load(const String<>& name) noexcept;

    void Play(const String<>& name, float speed = 1.f) noexcept;
    void Stop() noexcept;
    void SetVideoTimeline(const float timeStamp) noexcept;
    void StopAll() noexcept;
}
