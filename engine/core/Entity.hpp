#pragma once

#include <container/Pool.hpp>
#include <cstdint>

namespace mir {
    struct Id : public PoolHandle {
        constexpr Id() noexcept : PoolHandle {static_cast<std::size_t>(-1), 0 } {}
        constexpr Id(const PoolHandle& handle) noexcept : PoolHandle(handle) {}
        constexpr Id(std::size_t idx, std::size_t gen) noexcept : PoolHandle{ idx, gen } {}

        constexpr operator std::size_t() const noexcept {
            return Index;
        }
    };

    constexpr Id INVALID_ID{};
    constexpr std::uint16_t MAX_ID = UINT16_MAX;
}
