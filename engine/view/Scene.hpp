#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "../util/Debugger.hpp"
#include "../core/Manager.hpp"
#include "../util/Types.hpp"

namespace mir{
    namespace scene{
        inline Dictionary<String, Action<>> Scenes;
        inline String Current = "";
        inline String NextScene = "";
        inline String TargetScene = "";

        inline void Register(const String& name, std::function<void()> func){
            Scenes[name] = std::move(func);
        }

        inline void Update() {
            if (NextScene.empty()) return;
            if (Current == NextScene) {
                NextScene = "";
                return;
            }

            String name = NextScene;
            NextScene.clear();

            Dictionary<String, Action<>>::iterator iter = Scenes.find(name);
            if(iter == Scenes.end()) {
                debug::Log("Attempted to load an unregistered scene: %s", name.c_str());
                return;
            }

            debug::Log("Scene Transition: %s -> %s", Current.c_str(), name.c_str());
            Current = name;
            mir::Clear();
            iter->second();
        }

        inline void Load(const String& name, const String& loadingScene = "") {
            if(loadingScene.empty()){
                NextScene = name;
                TargetScene = "";
            }
            else{
                NextScene = loadingScene;
                TargetScene = name;
            }
        }
    }
}
