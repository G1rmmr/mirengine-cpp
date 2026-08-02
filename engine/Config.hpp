#pragma once

#include <cstddef>

#ifndef CONFIG_MAX_ENTITY
#define CONFIG_MAX_ENTITY 4096
#endif

#ifndef CONFIG_MAX_COMPONENT
#define CONFIG_MAX_COMPONENT 128
#endif

#ifndef CONFIG_MAX_SYSTEM
#define CONFIG_MAX_SYSTEM 64
#endif

#ifndef CONFIG_COMMAND_BUFFER_BYTES
#define CONFIG_COMMAND_BUFFER_BYTES (1024 * 1024)
#endif

namespace mir::config {
	constexpr std::size_t MAX_ENTITY = CONFIG_MAX_ENTITY;
	constexpr std::size_t MAX_COMPONENT = CONFIG_MAX_COMPONENT;
	constexpr std::size_t MAX_SYSTEM = CONFIG_MAX_SYSTEM;
	constexpr std::size_t COMMAND_BUFFER_BYTES = CONFIG_COMMAND_BUFFER_BYTES;
}
