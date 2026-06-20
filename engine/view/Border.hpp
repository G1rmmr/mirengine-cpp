#pragma once

#include <container/String.hpp>

using namespace zet;

namespace mir::border {
    void SetColor(const float r, const float g, const float b, const float a) noexcept;
    void SetPosition(const float x, const float y) noexcept;
    void SetSize(const float x, const float y) noexcept;
}
