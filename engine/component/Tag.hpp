#pragma once

#include <container/String.hpp>
#include "../core/Component.hpp"

namespace mir::tag {
    struct Tag : public Component<Tag, String<>> {};

    // Tags classify an entity and persist until explicitly changed or removed.
    [[nodiscard]] inline bool Set(const Id id, const String<>& tagName) noexcept {
        return Tag::Set(id, tagName);
    }
}
