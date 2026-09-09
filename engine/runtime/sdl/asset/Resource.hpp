#pragma once

#include <cstdint>
#include <container/String.hpp>

using namespace zet;

namespace mir::resource {
    constexpr std::uint16_t MAX_RESOURCE = UINT16_MAX;

    namespace {
        Map<String<>, String<>, MAX_RESOURCE> dictionary;
    }

    inline void Register(const String<>& name, const String<>& path) noexcept {
        dictionary[name] = path;
    }

    inline String<> GetPath(const String<>& name) noexcept { 
        return dictionary[name];
    }

    inline void Unregister(const String<>& name) noexcept { 
        dictionary.Delete(name);
    }

    void Clear() noexcept { 
        dictionary.Clear();
    }
}