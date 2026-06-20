#pragma once

#include <container/String.hpp>

using namespace zet;

namespace mir::button {
    void SetText(const String<>& text) noexcept;
    void SetFont(const String<>& font) noexcept;
    
    void SetBackgroundColor(const float r, const float g, const float b, const float a) noexcept;
    void SetForegroundColor(const float r, const float g, const float b, const float a) noexcept;
    
    void SetPosition(const float x, const float y) noexcept;
    void SetSize(const float width, const float height) noexcept;
}