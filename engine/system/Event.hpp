#pragma once

#include <container/String.hpp>
#include <container/Map.hpp>
#include "../component/Tag.hpp"

namespace mir::event {
	inline void Update() noexcept {
		for (const Id id : activeTagEntities) {
			if (!Tag::IsValidEntity(id)) continue;
			const String<>& tagName = Tag::Get(id);

			auto callbacks = tagCallbacks.Find(tagName);
			if (callbacks != nullptr) {
				for (const auto& callback : *callbacks) { callback(id); }
            }
            
            Tag::Remove(id);
        }
        
		activeTagEntities.Clear();
    }

    inline void OnTag(const String<>& tagName, CallbackFuncPtr callback) noexcept {
		auto callbacks = tagCallbacks.Find(tagName);
		if (callbacks == nullptr) {
			callbacks = &tagCallbacks.Insert(tagName);
		}
		callbacks->Push(callback);
    }
}
