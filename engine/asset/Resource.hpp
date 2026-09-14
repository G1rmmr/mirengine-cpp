#pragma once

#include <cstdint>
#include <container/String.hpp>
#include "../core/StringHash.hpp"
#include <container/Map.hpp>
#include <string_view>
#include <array>

using namespace zet;

namespace mir::resource {
	constexpr std::uint16_t MAX_RESOURCE = 1024;

	struct Entry {
		String<> Name;
		String<> Path;
	};

    // An inline variable has one program-wide instance. An unnamed namespace
    // here would create a separate dictionary for every translation unit.
    inline Map<String<>, String<>, MAX_RESOURCE> dictionary;
	inline std::array<Entry, MAX_RESOURCE> entries{};
	inline std::size_t entryCount = 0;

	inline void RebuildDictionary() noexcept {
		dictionary.Clear();
		for (std::size_t index = 0; index < entryCount; ++index) {
			dictionary.Insert(entries[index].Name, entries[index].Path);
		}
	}

    [[nodiscard]] inline bool Register(const String<>& name, const String<>& path) noexcept {
		if (name.Empty() || path.Empty()) return false;
		for (std::size_t index = 0; index < entryCount; ++index) {
			if (entries[index].Name == name) {
				entries[index].Path = path;
				dictionary.Insert(name, path);
				return true;
			}
		}
		if (entryCount >= MAX_RESOURCE) return false;
		entries[entryCount++] = Entry{name, path};
		dictionary.Insert(name, path);
		return true;
    }

    inline String<> GetPath(const String<>& name) noexcept { 
        auto ptr = dictionary.Find(name);
        return ptr ? *ptr : "";
    }

    [[nodiscard]] inline bool Unregister(const String<>& name) noexcept {
		for (std::size_t index = 0; index < entryCount; ++index) {
			if (entries[index].Name != name) continue;
			entries[index] = entries[entryCount - 1];
			--entryCount;
			RebuildDictionary();
			return true;
		}
		return false;
    }

	inline void Clear() noexcept {
		dictionary.Clear();
		entryCount = 0;
    }

	[[nodiscard]] inline std::size_t Count() noexcept { return entryCount; }
}
