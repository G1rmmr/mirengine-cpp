#pragma once

#include <functional>
#include <utility>

#include <container/String.hpp>
#include <container/Map.hpp>

#include "../util/Debugger.hpp"

using namespace zet;

namespace mir::scene {
    namespace {
        inline Map<String<>, std::function<void()>> scenes;
        inline String<> current = "";
    }
    
    inline void Register(const String<>& name, std::function<void()> func) {
        scenes[name] = std::move(func);
    }

    inline void Load(const String<>& sceneName) {
        if (current == sceneName) return;

        auto iter = scenes.Find(sceneName);
        if (iter == scenes.End()) {
            debug::Log("Attempted to load an unregistered scene: %s", sceneName.CStr());
            return;
        }

        debug::Log("Scene Transition: %s -> %s", Current.CStr(), name.CStr());

        current = sceneName;
        iter->second();
    }
}