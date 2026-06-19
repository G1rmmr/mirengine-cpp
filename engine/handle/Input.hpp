#pragma once

#include "Key.hpp"
#include <array>

namespace mir::input {
    constexpr std::size_t KEY_COUNT = static_cast<std::size_t>(Key::COUNT);

    inline std::array<bool, KEY_COUNT> currentStates{ false };
    inline std::array<bool, KEY_COUNT> previousStates{ false };

    inline std::array<bool, 3> currentMouseStates{ false };
    inline std::array<bool, 3> previousMouseStates{ false };

    inline float mouseX = 0.f;
    inline float mouseY = 0.f;

    inline void Update() {
        previousStates = currentStates;
        previousMouseStates = currentMouseStates;
    }

    inline bool IsPressed(const Key key) {
        return currentStates[static_cast<std::size_t>(key)];
    }

    inline bool IsJustPressed(const Key key) {
        std::size_t idx = static_cast<std::size_t>(key);
        return currentStates[idx] && !previousStates[idx];
    }

    inline bool IsJustReleased(const Key key) {
        std::size_t idx = static_cast<std::size_t>(key);
        return !currentStates[idx] && previousStates[idx];
    }

    inline bool IsMousePressed(const std::size_t button) {
        return button < 3 && currentMouseStates[button];
    }

    inline bool IsMouseJustPressed(const std::size_t button) {
        return button < 3 && currentMouseStates[button] && !previousMouseStates[button];
    }

    inline float GetMouseX() {
        return mouseX;
    }

    inline float GetMouseY() {
        return mouseY;
    }

    inline void SetKeyState(Key key, bool pressed) {
        if (key != Key::COUNT) {
            currentStates[static_cast<std::size_t>(key)] = pressed;
        }
    }

    inline void SetMouseState(std::size_t button, bool pressed) {
        if (button < 3) {
            currentMouseStates[button] = pressed;
        }
    }

    inline void SetMousePosition(float x, float y) {
        mouseX = x;
        mouseY = y;
    }
}
