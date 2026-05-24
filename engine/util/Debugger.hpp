#pragma once

#include <cstdio>
#include <utility>

#include "Timer.hpp"
#include "Types.hpp"

#ifdef DEBUG_MODE
    static constexpr mir::Bool DEBUG_ENABLED = true;
#else
    static constexpr mir::Bool DEBUG_ENABLED = false;
#endif

namespace mir {
    namespace debug {
        static inline std::unique_ptr<Font> Font;
        static inline std::unique_ptr<Text> Text;

        inline Bool IsEntityCountVisible = false;
        inline Bool IsColliderVisible = false;

        template<typename... Args>
        static inline void Log(const char* format, Args&&... args){
            if constexpr(!DEBUG_ENABLED) return;

            time::Set nowLocalTimeSet = time::GetLocalTime();
            std::fprintf(stdout, "%02d:%02d:%02d.%03d\t",
                nowLocalTimeSet.Hour,
                nowLocalTimeSet.Minute,
                nowLocalTimeSet.Second,
                nowLocalTimeSet.MiliSec);

            if constexpr(sizeof...(Args) == 0) std::printf("%s", format);
            else std::printf(format, std::forward<Args>(args)...);

            std::printf("\n");
            std::fflush(stdout);
        }

        static inline void ToggleDebug(){
            if constexpr(!DEBUG_ENABLED) return;
            IsColliderVisible = !IsColliderVisible;
            IsEntityCountVisible = !IsEntityCountVisible;
        }
    }
}
