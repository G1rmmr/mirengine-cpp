#pragma once

#include <cstdint>
#include <memory/PointHandle.hpp>

using namespace zet;

namespace mir::window {
    enum class Mode {
        Windowed,
        Fullscreen,
        Borderless,
        Desktop
    };

    enum class Resolution {
        HD,  // 1280x720
        FHD, // 1920x1080
        QHD, // 2560x1440
        UHD, // 3840x2160
        Custom
    };

    inline String<> Title;
    inline std::uint16_t ScreenWidth;
    inline std::uint16_t ScreenHeight;
    
    // 윈도우 인스턴스를 가리키는 추상 핸들
    inline PointHandle<void> WindowInstance = nullptr;

    PointHandle<void> GetWindow() noexcept;
    bool IsOpen() noexcept;
    bool IsOpening() noexcept;

    void Init(
        const String<>& title,
        Mode mode = Mode::Desktop,
        Resolution res = Resolution::FHD,
        uint32_t width = 0,
        uint32_t height = 0) noexcept;

    void ProcessEvents() noexcept;
    void Render() noexcept;
    void Display() noexcept;
    void Clear(std::uint8_t red = 0, std::uint8_t green = 0, std::uint8_t blue = 0) noexcept;
    void Close() noexcept;
    void Shutdown() noexcept;

    void SetTitle(const String<>& title) noexcept;
    void SetFPS(const std::uint16_t fps) noexcept;
    void SetSize(const std::uint16_t width, const std::uint16_t height) noexcept;
    void SetMode(const Mode mode) noexcept;
    void SetResolution(const Resolution resolution) noexcept;
    
    Point2<float> MapPixelToCoords(const Point2<int>& pixel) noexcept;
}