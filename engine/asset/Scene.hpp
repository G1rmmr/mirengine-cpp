#pragma once

#include <functional>
#include <utility>

#include <container/String.hpp>
#include "../core/StringHash.hpp"
#include <container/Map.hpp>

#include "../util/Debugger.hpp"

using namespace zet;

namespace mir::scene {
    inline Map<String<>, std::function<void()>> scenes;
    inline String<> current = "";
    
	inline void Register(const String<>& name, std::function<void()> func) {
		scenes.Insert(name, std::move(func));
    }

    inline void Load(const String<>& sceneName) {
        if (current == sceneName) return;

		auto scene = scenes.Find(sceneName);
		if (scene == nullptr) {
            debug::Log("Attempted to load an unregistered scene: %s", sceneName.CStr());
            return;
        }

        debug::Log("Scene Transition: %s -> %s", current.CStr(), sceneName.CStr());

        current = sceneName;
		(*scene)();
    }
}
