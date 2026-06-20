#pragma once

#include <container/String.hpp>
#include <container/Map.hpp>
#include <container/List.hpp>

#include "../core/Component.hpp"

namespace mir::event {
    using CallbackFuncPtr = void(*)(Id);

    inline Map<String<>, List<CallbackFuncPtr>> tagCallbacks;
    inline List<Id> activeTagEntities;

    struct Tag : public Component<Tag, String<>> {};

    inline void SetTag(const Id id, const String<>& tagName) noexcept {
        Tag::Set(id, tagName);
        activeTagEntities.Push(id);
    }
}