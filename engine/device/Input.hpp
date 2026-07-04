#pragma once

#include "Key.hpp"
#include <array>

namespace mir::input {
    constexpr std::size_t KEY_COUNT = static_cast<std::size_t>(Key::COUNT);

    inline std::array<bool, KEY_COUNT> CurrentStates{};
    inline std::array<bool, KEY_COUNT> PreviousStates{};

    inline std::array<bool, 3> CurrentMouseStates{};
    inline std::array<bool, 3> PreviousMouseStates{};

    inline float MouseX = 0.f;
    inline float MouseY = 0.f;

    inline void Update() {
        PreviousStates = CurrentStates;
        PreviousMouseStates = CurrentMouseStates;
    }

    inline bool IsPressed(const Key key) {
        return CurrentStates[static_cast<std::size_t>(key)];
    }

    inline bool IsJustPressed(const Key key) {
        std::size_t idx = static_cast<std::size_t>(key);
        return CurrentStates[idx] && !PreviousStates[idx];
    }

    inline bool IsJustReleased(const Key key) {
        std::size_t idx = static_cast<std::size_t>(key);
        return !CurrentStates[idx] && PreviousStates[idx];
    }

    inline bool IsMousePressed(const std::size_t button) {
        return button < 3 && CurrentMouseStates[button];
    }

    inline bool IsMouseJustPressed(const std::size_t button) {
        return button < 3 && CurrentMouseStates[button] && !PreviousMouseStates[button];
    }

    inline float GetMouseX() {
        return MouseX;
    }

    inline float GetMouseY() {
        return MouseY;
    }

    inline void SetKeyState(Key key, bool pressed) {
        if (key != Key::COUNT) {
            CurrentStates[static_cast<std::size_t>(key)] = pressed;
        }
    }

    inline void SetMouseState(std::size_t button, bool pressed) {
        if (button < 3) {
            CurrentMouseStates[button] = pressed;
        }
    }

    inline void SetMousePosition(float x, float y) {
        MouseX = x;
        MouseY = y;
    }
}
