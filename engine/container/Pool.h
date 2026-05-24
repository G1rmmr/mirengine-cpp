#pragma once

#include <utility>
#include <cassert>
#include <memory>
#include <concepts>
#include "Defines.h"

namespace mir {
    struct PoolHandle {
        std::size_t index;
        std::size_t generation;

        constexpr bool operator==(const PoolHandle& other) const noexcept {
            return index == other.index && generation == other.generation;
        }
        
        constexpr bool operator!=(const PoolHandle& other) const noexcept {
            return !(*this == other);
        }
    };

    static constexpr PoolHandle INVALID_POOL_HANDLE = { static_cast<std::size_t>(-1), 0 };

    template <typename T, std::size_t C = DEFAULT_POOL_CAPACITY> requires (C > 0) && std::destructible<T>
    class Pool {
    public:
        class Iterator {
        public:
            constexpr Iterator(Pool* pool, std::size_t index) : pool(pool), index(index) {
                MoveToNext();
            }

            constexpr T& operator*() const {
                return pool->data[index].value;
            }

            constexpr Iterator& operator++() {
                ++index;
                MoveToNext();
                return *this;
            }

            constexpr bool operator==(const Iterator& other) const {
                return index == other.index;
            }

            constexpr bool operator!=(const Iterator& other) const {
                return !(*this == other);
            }

        private:
            constexpr void MoveToNext() {
                while (index < C && !pool->occupied[index]) {
                    ++index;
                }
            }

            Pool* pool;
            std::size_t index;
        };

        constexpr Pool() : nextFree(0) {
            for (std::size_t i = 0; i < C; ++i) {
                occupied[i] = false;
                generations[i] = 1; // 세대는 1부터 시작
                if (i < C - 1) {
                    data[i].next = i + 1;
                }
            }
            data[C - 1].next = TERMINATOR;
        }

        constexpr ~Pool() {
            Clear();
        }

        Pool(const Pool&) = delete;
        Pool& operator=(const Pool&) = delete;

        constexpr Iterator begin() { return Iterator(this, 0); }
        constexpr Iterator end() { return Iterator(this, C); }

        template <typename... Args> requires std::constructible_from<T, Args...>
        constexpr PoolHandle Create(Args&&... args) {
            assert(nextFree != TERMINATOR && "[mir::Pool] POOL IS FULL");

            std::size_t index = nextFree;
            nextFree = data[index].next;

            std::construct_at(std::addressof(data[index].value), std::forward<Args>(args)...);
            occupied[index] = true;
            
            return PoolHandle{ index, generations[index] };
        }

        constexpr void Destroy(PoolHandle handle) {
            assert(handle.index < C && "[mir::Pool] INVALID INDEX");

            assert(generations[handle.index] == handle.generation && 
                "[mir::Pool] DANGLING HANDLE (ALREADY DESTROYED OR REPLACED)");

            assert(occupied[handle.index] && "[mir::Pool] DESTROYING NON-EXISTENT OBJECT");

            std::destroy_at(std::addressof(data[handle.index].value));
            
            occupied[handle.index] = false;
            generations[handle.index]++;
            
            data[handle.index].next = nextFree;
            nextFree = handle.index;
        }

        constexpr T& Get(PoolHandle handle) {
            assert(handle.index < C && "[mir::Pool] INVALID INDEX");

            assert(generations[handle.index] == handle.generation && 
                "[mir::Pool] DANGLING HANDLE DETECTED (USE AFTER FREE)");

            assert(occupied[handle.index] && "[mir::Pool] ACCESSING NON-EXISTENT OBJECT");
            
            return data[handle.index].value;
        }

        constexpr const T& Get(PoolHandle handle) const {
            assert(handle.index < C && "[mir::Pool] INVALID INDEX");

            assert(generations[handle.index] == handle.generation && 
                "[mir::Pool] DANGLING HANDLE DETECTED (USE AFTER FREE)");

            assert(occupied[handle.index] && "[mir::Pool] ACCESSING NON-EXISTENT OBJECT");
            
            return data[handle.index].value;
        }

        constexpr T& operator[](PoolHandle handle) { 
            return Get(handle); 
        }

        constexpr const T& operator[](PoolHandle handle) const { 
            return Get(handle); 
        }

        constexpr void Clear() {
            for (std::size_t i = 0; i < C; ++i) {
                if (occupied[i]) {
                    std::destroy_at(std::addressof(data[i].value));
                    occupied[i] = false;
                    generations[i]++;
                }
            }
            
            nextFree = 0;
            for (std::size_t i = 0; i < C; ++i) {
                if (i < C - 1) {
                    data[i].next = i + 1;
                }
            }
            data[C - 1].next = TERMINATOR;
        }

    private:
        static constexpr std::size_t TERMINATOR = static_cast<std::size_t>(-1);

        union Node {
            T value;
            std::size_t next;

            constexpr Node() : next(0) {}
            constexpr ~Node() {}
        };

        Node data[C];
        std::size_t generations[C];
        std::size_t nextFree;
        bool occupied[C];
    };
}