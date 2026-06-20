#pragma once

#include <container/String.hpp>
#include <container/Map.hpp>
#include <container/List.hpp>

#include <memory/PointHandle.hpp>

#include "../component/Tag.hpp"

namespace mir::event {
    inline void Update() noexcept {
        for (const Id id : activeTagEntities) {
            const String<>& tagName = Tag::Get(id);

            auto it = tagCallbacks.find(tagName);
            if (it != tagCallbacks.end()) {
                for (const auto& callback : it->second) { callback(id); }
            }
            
            Tag::Remove(id);
        }
        
        activeTagEntities.clear();
    }

    inline void OnTag(const String<>& tagName, CallbackFuncPtr callback) noexcept {
        tagCallbacks[tagName].push_back(std::move(callback));
    }
}