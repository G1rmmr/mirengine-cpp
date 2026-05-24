#pragma once

#include <type_traits>
#include <utility>
#include <cassert>
#include <memory>
#include <concepts>
#include "Defines.h"

namespace mir {
    template <typename T, std::size_t C = DEFAULT_ARRAY_CAPACITY> requires (C > 0) && std::destructible<T>
    class Array {
    public:
        constexpr Array() : size(0) {}

        constexpr ~Array() {
            Clear();
        }

        constexpr Array(const Array<T, C>& other) requires std::copy_constructible<T> : size(0) {
            for (std::size_t i = 0; i < other.size; ++i) {
                Push(other[i]);
            }
        }

        constexpr Array& operator=(const Array& other) requires std::copy_constructible<T> {
            if (this != &other) {
                Clear();
                for (std::size_t i = 0; i < other.size; ++i) {
                    Push(other[i]);
                }
            }
            return *this;
        }

        constexpr Array(Array&& other) noexcept requires std::move_constructible<T> : size(0) {
            for (std::size_t i = 0; i < other.size; ++i) {
                Push(std::move(other[i]));
            }
            other.Clear();
        }

        constexpr Array& operator=(Array&& other) noexcept requires std::move_constructible<T> {
            if (this != &other) {
                Clear();
                for (std::size_t i = 0; i < other.size; ++i) {
                    Push(std::move(other[i]));
                }
                other.Clear();
            }
            return *this;
        }

        template <typename... Args> requires std::constructible_from<T, Args...>
        constexpr T& Push(Args&&... args) {
            assert(size < C && "[mir::Array] DATA IS FULL");
            T* ptr = std::construct_at(std::addressof(data[size++].value), std::forward<Args>(args)...);
            return *ptr;
        }

        constexpr void Push(const T& value) requires std::copy_constructible<T> {
            Push(value); 
        }

        constexpr void Push(T&& value) requires std::move_constructible<T> {
            Push(std::move(value)); 
        }

        constexpr void Pop() {
            assert(size > 0 && "[mir::Array] DATA IS EMPTY");
            std::destroy_at(std::addressof(data[--size].value));
        }

        constexpr void Clear() {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                while (size > 0) {
                    Pop();
                }
            }
            else {
                size = 0;
            }
        }

        constexpr T& operator[](std::size_t index) {
            assert(index < size && "[mir::Array] INDEX OUT OF BOUNDS");
            return *std::addressof(data[index].value);
        }

        constexpr const T& operator[](std::size_t index) const {
            assert(index < size && "[mir::Array] INDEX OUT OF BOUNDS");
            return *std::addressof(data[index].value);
        }

        constexpr T* begin() noexcept { 
            return std::addressof(data[0].value); 
        }

        constexpr const T* begin() const noexcept { 
            return std::addressof(data[0].value); 
        }

        constexpr T* end() noexcept { 
            return std::addressof(data[size].value); 
        }

        constexpr const T* end() const noexcept { 
            return std::addressof(data[size].value); 
        }

        constexpr std::size_t Size() const noexcept { 
            return size; 
        }

        static constexpr std::size_t Capacity() noexcept { 
            return C; 
        }

    private:
        union Storage {
            T value;
            constexpr Storage() {}
            constexpr ~Storage() {}
        };

        Storage data[C];
        std::size_t size;
    };
}