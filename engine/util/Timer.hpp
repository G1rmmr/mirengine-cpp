#pragma once

#include <chrono>
#include <ctime>
#include <functional>
#include <utility>

#include <container/List.hpp>

using namespace zet;

namespace mir::time {
    struct TimerTask {
        std::function<void()> CallBack;
        float IntervalSec;
        float RemainSec;
        bool IsLooping;
    };

    namespace {
        static inline List<TimerTask> TimerTasks;
    }

    struct Set {
        int Hour;
        int Minute;
        int Second;
        int MiliSec;
    };

    inline Set GetLocalTime() noexcept {
        const auto now = std::chrono::system_clock::now();
        const std::time_t time = std::chrono::system_clock::to_time_t(now);
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::tm localTime;
#if defined(_MSC_VER)
        localtime_s(&localTime, &time);
#else
        localtime_r(&time, &localTime);
#endif

        return {
            localTime.tm_hour,
            localTime.tm_min,
            localTime.tm_sec,
            static_cast<int>(ms.count())
        };
    }

    // Platform-independent and graphics-library-independent delta time measurement
    inline float GetDelta() noexcept {
        using clock = std::chrono::high_resolution_clock;
        static auto lastTime = clock::now();
        auto currentTime = clock::now();
        std::chrono::duration<float> delta = currentTime - lastTime;
        lastTime = currentTime;
        return delta.count();
    }

    inline void Register(const float seconds, std::function<void()> callback, bool isLooping = false) noexcept {
        TimerTasks.Push({std::move(callback), seconds, seconds, isLooping});
    }

    inline void Update(const float deltaTime) noexcept {
        if (TimerTasks.Empty()) return;

        List<TimerTask> nextTasks;
        for (std::size_t i = 0; i < TimerTasks.Size(); ++i) {
            TimerTasks[i].RemainSec -= deltaTime;

            if (TimerTasks[i].RemainSec <= 0.f) {
                if (TimerTasks[i].CallBack) {
                    TimerTasks[i].CallBack();
                }
                if (TimerTasks[i].IsLooping) {
                    TimerTasks[i].RemainSec += TimerTasks[i].IntervalSec;
                    nextTasks.Push(std::move(TimerTasks[i]));
                }
            } else {
                nextTasks.Push(std::move(TimerTasks[i]));
            }
        }
        TimerTasks = std::move(nextTasks);
    }
}
