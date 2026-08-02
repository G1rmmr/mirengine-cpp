#pragma once

#include <cstdint>
#include <container/String.hpp>
#include <container/Map.hpp>
#include <string_view>
#include "../core/StringHash.hpp"

using namespace zet;

namespace mir::resource {
	constexpr std::uint16_t MAX_RESOURCE = 1024;

    namespace {
        Map<String<>, String<>, MAX_RESOURCE> dictionary;
    }

    inline void Register(const String<>& name, const String<>& path) noexcept {
        dictionary.Insert(name, path);
    }

    inline String<> GetPath(const String<>& name) noexcept { 
        auto ptr = dictionary.Find(name);
        return ptr ? *ptr : "";
    }

    inline void Unregister(const String<>& name) noexcept { 
        // zet::Map does not support deleting a single element
    }

	inline void Clear() noexcept {
        dictionary.Clear();
    }
}
