#pragma once

#include <container/Pool.hpp>
#include <cstdint>
#include "../Config.hpp"

using namespace zet;

namespace mir {
    struct Id : public PoolHandle {
        constexpr Id() noexcept : PoolHandle{} {}
        constexpr Id(const PoolHandle& handle) noexcept : PoolHandle(handle) {}

        constexpr operator std::size_t() const noexcept {
            return Index;
        }
    };

    constexpr Id INVALID_ID{};
#ifdef CONFIG_MAX_ENTITY
    constexpr std::uint32_t MAX_ID = CONFIG_MAX_ENTITY;
#else
    constexpr std::uint32_t MAX_ID = UINT32_MAX;
#endif
}
