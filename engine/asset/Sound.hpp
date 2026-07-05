#pragma once

#include <container/String.hpp>

using namespace zet;

namespace mir::sound {
    bool Load(const String<>& name) noexcept;

    void Play(const String<>& name, float volume = 100.f, float pitch = 1.f) noexcept;
    void PlayAt(const String<>& name, const float x, const float y, float volume = 100.f, float pitch = 1.f) noexcept;

    void PlayBgm(const String<>& name, float volume = 100.f, bool loop = true) noexcept;
    void StopBgm() noexcept;
    void SetBgmVolume(float volume) noexcept;

    void SetMasterVolume(float volume) noexcept;
    void StopAll() noexcept;
}
