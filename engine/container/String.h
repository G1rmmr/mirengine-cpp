#pragma once

#include <cstddef>
#include <string_view>
#include <cassert>
#include "Defines.h"

namespace mir {
    template <std::size_t C = DEFAULT_STRING_CAPACITY> requires (C > 0)
    class String {
    public:
        constexpr String() noexcept : length(0) {
            buffer[0] = '\0';
        }

        constexpr String(const char* str) {
            length = 0;
            while (str && str[length] != '\0' && length < C - 1) {
                buffer[length] = str[length];
                length++;
            }
            buffer[length] = '\0';
        }

        constexpr String(std::string_view sv) {
            length = 0;
            for (char c : sv) {
                if (length >= C - 1) break;
                buffer[length++] = c;
            }
            buffer[length] = '\0';
        }

        constexpr String(std::size_t count, char ch) : length(0) {
            while (length < count && length < C - 1) {
                buffer[length++] = ch;
            }
            buffer[length] = '\0';
        }

        constexpr std::size_t size() const noexcept { 
            return length; 
        }

        constexpr std::size_t capacity() const noexcept { 
            return C - 1; 
        }

        constexpr bool empty() const noexcept { 
            return length == 0; 
        }

        constexpr const char* c_str() const noexcept { 
            return buffer; 
        }

        constexpr char* data() noexcept { 
            return buffer; 
        }

        constexpr const char* data() const noexcept { 
            return buffer; 
        }
        
        constexpr std::size_t Size() const noexcept { 
            return size(); 
        }

        constexpr std::size_t Capacity() const noexcept { 
            return capacity(); 
        }

        constexpr bool Empty() const noexcept { 
            return empty(); 
        }

        constexpr const char* CStr() const noexcept { 
            return c_str(); 
        }

        constexpr std::string_view View() const noexcept { 
            return std::string_view(buffer, length); 
        }

        constexpr char& operator[](std::size_t idx) {
            assert(idx < length && "[mir::String] ICDEX OUT OF BOUCDS");
            return buffer[idx];
        }

        constexpr const char& operator[](std::size_t idx) const {
            assert(idx < length && "[mir::String] ICDEX OUT OF BOUCDS");
            return buffer[idx];
        }

        constexpr bool operator==(const String& other) const noexcept {
            if (length != other.length) {
                return false;
            }

            for (std::size_t i = 0; i < length; ++i) {
                if (buffer[i] != other.buffer[i]) {
                    return false;   
                }
            }
            return true;
        }

        constexpr bool operator==(std::string_view other) const noexcept {
            return View() == other;
        }

        constexpr bool operator==(const char* other) const noexcept {
            return View() == std::string_view(other);
        }

        constexpr String& operator+=(const char* str) {
            while (str && *str != '\0' && length < C - 1) {
                buffer[length++] = *str++;
            }

            buffer[length] = '\0';
            return *this;
        }

        constexpr String& operator+=(std::string_view stringView) {
            for (char c : stringView) {
                if (length >= C - 1) {
                    break;
                }

                buffer[length++] = c;
            }

            buffer[length] = '\0';
            return *this;
        }

        constexpr String operator+(const String& other) const {
            String result(*this);
            result += other.View();
            return result;
        }
        
        constexpr String operator+(const char* str) const {
            String result(*this);
            result += str;
            return result;
        }

        constexpr operator std::string_view() const noexcept {
            return View();
        }

    private:
        char buffer[C];
        std::size_t length;
    };
}