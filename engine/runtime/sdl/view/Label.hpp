#pragma once

#include <container/String.hpp>

using namespace zet;

namespace mir::label {
    void SetText(const String<>& text) noexcept;
    void SetFont(const String<>& font) noexcept;
    void SetSize(const float size) noexcept;
    void SetColor(const float r, const float g, const float b, const float a) noexcept;
    void SetPosition(const float x, const float y) noexcept;
}
