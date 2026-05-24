#pragma once

#include <chrono>

#include "Debugger.hpp"
#include "Types.hpp"

namespace mir{
    namespace profile{
        static inline std::unique_ptr<Font> Font;
        static inline std::unique_ptr<Text> Text;
        static inline Bool IsEnable = false;

        inline Real CurrentFPS = 0;

        class ScopeTimer{
        public:
            ScopeTimer(std::string_view name) : boundName(name){
                if(IsEnable) start = std::chrono::high_resolution_clock::now();
            }

            ScopeTimer(const ScopeTimer& other) = default;
            ScopeTimer& operator=(const ScopeTimer& other) = default;

            ScopeTimer(ScopeTimer&& other) noexcept = default;
            ScopeTimer& operator=(ScopeTimer&& other) noexcept = default;

            ~ScopeTimer(){
                if(!IsEnable) return;

                std::chrono::high_resolution_clock::time_point end
                    = std::chrono::high_resolution_clock::now();

                std::chrono::microseconds duration
                    = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

                debug::Log("[PROFILE] %s: %lld μs", boundName, duration.count());
            }
        private:
            std::chrono::high_resolution_clock::time_point start;
            std::string_view boundName;
        };

        static inline void ToggleProfile(){ IsEnable = !IsEnable; }
        static inline void Update(const Real deltaTime){
            if(IsEnable) CurrentFPS = 1 / deltaTime;
        }
    }
}

#define PROFILE_SCOPE(name) if(mir::profile::ScopeTimer _timer##__LINE__(name); true)
