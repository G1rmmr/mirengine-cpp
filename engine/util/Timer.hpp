#pragma once

#include <chrono>
#include <ctime>
#include <functional>
#include <utility>
#include <array>
#include <cstddef>

#include "../core/Manager.hpp"

namespace mir::time {
	inline constexpr std::size_t MAX_TIMER_TASKS = 256;

	struct TimerHandle {
		std::size_t Index = MAX_TIMER_TASKS;
		std::size_t Generation = 0;

		constexpr bool operator==(const TimerHandle&) const noexcept = default;
	};

    struct TimerTask {
        std::function<void()> CallBack;
        float IntervalSec = 0.f;
        float RemainSec = 0.f;
        bool IsLooping = false;
		bool Active = false;
		std::size_t Generation = 0;
    };

	inline std::array<TimerTask, MAX_TIMER_TASKS> TimerTasks{};
	inline core::SystemId TimerSystem{};

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

	inline void Update(const float deltaTime);

	inline void UpdateSystem(const float deltaTime) {
		Update(deltaTime);
    }

	[[nodiscard]] inline bool EnsureSystem() noexcept {
		auto& manager = core::Manager::Instance();
		if (manager.IsValidSystem(TimerSystem)) return true;
		TimerSystem = manager.RegisterSystem(&UpdateSystem, core::SystemPhase::Simulation);
		return manager.IsValidSystem(TimerSystem);
	}

	[[nodiscard]] inline TimerHandle Register(const float seconds, std::function<void()> callback, const bool isLooping = false) {
		if (seconds <= 0.f || !callback || !EnsureSystem()) return {};
		for (std::size_t index = 0; index < MAX_TIMER_TASKS; ++index) {
			TimerTask& task = TimerTasks[index];
			if (task.Active) continue;
			++task.Generation;
			if (task.Generation == 0) ++task.Generation;
			task.CallBack = std::move(callback);
			task.IntervalSec = seconds;
			task.RemainSec = seconds;
			task.IsLooping = isLooping;
			task.Active = true;
			return TimerHandle{index, task.Generation};
		}
		return {};
	}

	[[nodiscard]] inline bool IsValid(const TimerHandle handle) noexcept {
		return handle.Index < MAX_TIMER_TASKS && TimerTasks[handle.Index].Active &&
			TimerTasks[handle.Index].Generation == handle.Generation;
	}

	[[nodiscard]] inline bool Cancel(const TimerHandle handle) noexcept {
		if (!IsValid(handle)) return false;
		TimerTask& task = TimerTasks[handle.Index];
		task.Active = false;
		task.CallBack = {};
		return true;
	}

	inline void Clear() noexcept {
		for (TimerTask& task : TimerTasks) {
			task.Active = false;
			task.CallBack = {};
		}
	}

	inline void Update(const float deltaTime) {
		if (deltaTime < 0.f) return;
		for (TimerTask& task : TimerTasks) {
			if (!task.Active) continue;
			task.RemainSec -= deltaTime;
			if (task.RemainSec > 0.f) continue;

			const bool loop = task.IsLooping;
			const float interval = task.IntervalSec;
			if (!loop) task.Active = false;
			if (task.CallBack) task.CallBack();
			if (loop && task.Active) {
				task.RemainSec += interval;
				while (task.RemainSec <= 0.f) task.RemainSec += interval;
			} else if (!task.Active) {
				task.CallBack = {};
			}
		}
    }
}
