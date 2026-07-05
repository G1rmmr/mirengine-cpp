#pragma once

#include <container/Pool.hpp>
#include <cstdint>

using namespace zet;

namespace mir {
    struct Id : public PoolHandle {
        constexpr Id() noexcept : PoolHandle{static_cast<std::size_t>(-1), 0} {}
        constexpr Id(const PoolHandle& handle) noexcept : PoolHandle(handle) {}
        constexpr Id(const std::size_t idx, const std::size_t gen) noexcept : PoolHandle{idx, gen} {}

        constexpr operator std::size_t() const noexcept {
            return Index;
        }
    };

    constexpr Id INVALID_ID{};
#ifdef CONFIG_MAX_ENTITY
    constexpr std::uint16_t MAX_ID = CONFIG_MAX_ENTITY;
#else
    constexpr std::uint16_t MAX_ID = UINT16_MAX;
#endif
}
