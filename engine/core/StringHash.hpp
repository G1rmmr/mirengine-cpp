#pragma once

#include <container/String.hpp>
#include <functional>
#include <string_view>

namespace std {
	template <std::size_t C>
	struct hash<mir::String<C>> {
		std::size_t operator()(const mir::String<C>& value) const noexcept {
			return std::hash<std::string_view>{}(value.View());
		}
	};
}
