#pragma once

#include <functional>
#include <utility>
#include <array>

#include <container/String.hpp>
#include "../core/StringHash.hpp"
#include <container/Map.hpp>

#include "../util/Debugger.hpp"

using namespace zet;

namespace mir::scene {
	inline constexpr std::size_t MAX_SCENES = 128;
	struct Entry {
		String<> Name;
		std::function<void()> Callback;
	};
	inline std::array<Entry, MAX_SCENES> scenes{};
	inline std::size_t sceneCount = 0;
    inline String<> current = "";
    
	[[nodiscard]] inline bool Register(const String<>& name, std::function<void()> func) {
		if (name.Empty() || !func) return false;
		for (std::size_t index = 0; index < sceneCount; ++index) {
			if (scenes[index].Name == name) {
				scenes[index].Callback = std::move(func);
				return true;
			}
		}
		if (sceneCount >= MAX_SCENES) return false;
		scenes[sceneCount++] = Entry{name, std::move(func)};
		return true;
    }

    [[nodiscard]] inline bool Load(const String<>& sceneName) {
        if (current == sceneName) return true;

		for (std::size_t index = 0; index < sceneCount; ++index) {
			if (scenes[index].Name != sceneName) continue;
			debug::Log("Scene Transition: %s -> %s", current.CStr(), sceneName.CStr());
			current = sceneName;
			scenes[index].Callback();
			return true;
		}
		{
            debug::Log("Attempted to load an unregistered scene: %s", sceneName.CStr());
            return false;
        }
    }

	[[nodiscard]] inline bool Unregister(const String<>& name) {
		for (std::size_t index = 0; index < sceneCount; ++index) {
			if (scenes[index].Name != name) continue;
			scenes[index] = std::move(scenes[sceneCount - 1]);
			--sceneCount;
			if (current == name) current = "";
			return true;
		}
		return false;
	}

	inline void Clear() {
		for (std::size_t index = 0; index < sceneCount; ++index) scenes[index].Callback = {};
		sceneCount = 0;
		current = "";
	}

	[[nodiscard]] inline const String<>& Current() noexcept { return current; }
	[[nodiscard]] inline std::size_t Count() noexcept { return sceneCount; }
}
