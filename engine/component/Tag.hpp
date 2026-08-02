#pragma once

#include <container/String.hpp>
#include <container/Map.hpp>
#include <container/List.hpp>

#include "../core/Component.hpp"
#include "../core/StringHash.hpp"

namespace mir::event {
	using CallbackFuncPtr = void(*)(Id);
	inline constexpr std::size_t MAX_TAGS = 256;
	inline constexpr std::size_t MAX_CALLBACKS_PER_TAG = 16;

	inline Map<String<>, List<CallbackFuncPtr, MAX_CALLBACKS_PER_TAG>, MAX_TAGS> tagCallbacks;
	inline List<Id, MAX_ID> activeTagEntities;

    struct Tag : public Component<Tag, String<>> {};

	inline void SetTag(const Id id, const String<>& tagName) noexcept {
		if (!Tag::Set(id, tagName)) return;
		for (const Id active : activeTagEntities) {
			if (active == id) return;
		}
		activeTagEntities.Push(id);
    }
}
