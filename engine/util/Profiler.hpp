#pragma once

#include <chrono>
#include <container/String.hpp>

#include "Debugger.hpp"

namespace mir::profile {
    inline bool IsEnable = false;

    inline float CurrentFPS = 0.f;

    class ScopeTimer {
    public:
        ScopeTimer(std::string_view name) : boundName(name) {
            if (IsEnable) start = std::chrono::high_resolution_clock::now();
        }

        ScopeTimer(const ScopeTimer& other) = default;
        ScopeTimer& operator=(const ScopeTimer& other) = default;

        ScopeTimer(ScopeTimer&& other) noexcept = default;
        ScopeTimer& operator=(ScopeTimer&& other) noexcept = default;

        ~ScopeTimer() {
            if (!IsEnable) return;

            std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

            std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

            debug::Log("[PROFILE] %s: %lld μs", boundName, duration.count());
        }

    private:
        std::chrono::high_resolution_clock::time_point start;
        zet::String<> boundName;
    };

    inline void ToggleProfile() { IsEnable = !IsEnable; }

    inline void Update(const float deltaTime) {
        if (IsEnable) CurrentFPS = 1.f / deltaTime;
    }
}

#define PROFILE_SCOPE(name) if (mir::profile::ScopeTimer _timer##__LINE__(name); true)
