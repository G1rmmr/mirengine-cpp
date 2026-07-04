#pragma once

#include <cstdint>
#include <container/String.hpp>
#include <container/Map.hpp>
#include <string_view>

using namespace zet;

namespace std {
    template <std::size_t C>
    struct hash<mir::String<C>> {
        std::size_t operator()(const mir::String<C>& str) const noexcept {
            return std::hash<std::string_view>{}(std::string_view(str));
        }
    };
}

namespace mir::resource {
    constexpr std::uint16_t MAX_RESOURCE = UINT16_MAX;

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

    void Clear() noexcept { 
        dictionary.Clear();
    }
}