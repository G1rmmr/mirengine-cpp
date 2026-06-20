#pragma once

#include <memory/PointerHandle.hpp>
#include <utility>
#include <cstdio>

#include "Timer.hpp"

#ifdef DEBUG_MODE
static constexpr bool DEBUG_ENABLED = true;
#else
static constexpr bool DEBUG_ENABLED = false;
#endif

namespace mir::debug {
    inline bool IsEntityCountVisible = false;
    inline bool IsColliderVisible = false;

    template <typename... Args> inline void Log(const char* format, Args&&... args) {
        if constexpr (!DEBUG_ENABLED) { return; }

        time::Set nowLocalTimeSet = time::GetLocalTime();

        std::fprintf(stdout, "%02d:%02d:%02d.%03d\t", nowLocalTimeSet.Hour, nowLocalTimeSet.Minute,
                     nowLocalTimeSet.Second, nowLocalTimeSet.MiliSec);

        if constexpr (sizeof...(Args) == 0) {
            std::printf("%s", format);
        } else {
            std::printf(format, std::forward<Args>(args)...);
        }

        std::printf("\n");
        std::fflush(stdout);
    }

    inline void ToggleDebug() {
        if constexpr (!DEBUG_ENABLED) { return; }

        IsColliderVisible = !IsColliderVisible;
        IsEntityCountVisible = !IsEntityCountVisible;
    }
}
