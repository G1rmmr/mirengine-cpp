#include "SDLInternal.hpp"
#include "device/Window.hpp"
#include "device/Input.hpp"
#include "device/Key.hpp"
#include "core/Manager.hpp"
#include "component/Sprite.hpp"
#include "component/Transform.hpp"
#include "view/Label.hpp"
#include "view/Border.hpp"
#include "view/Button.hpp"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <cstdio>

namespace mir::sdl {
    SDL_Window* sdlWindow = nullptr;
    SDL_Renderer* sdlRenderer = nullptr;
    MIX_Mixer* sdlMixer = nullptr;
    static bool isWindowOpen = false;
    static std::uint16_t fpsLimit = 60;
    static std::uint64_t frameDelay = 1000 / 60;
    static std::uint64_t lastFrameTime = 0;

    static mir::Key MapSDLKey(SDL_Keycode sym) {
        switch (sym) {
            case SDLK_ESCAPE: return mir::Key::Escape;
            case SDLK_SPACE: return mir::Key::Space;
            case SDLK_RETURN: return mir::Key::Enter;
            case SDLK_UP: return mir::Key::Up;
            case SDLK_LEFT: return mir::Key::Left;
            case SDLK_DOWN: return mir::Key::Down;
            case SDLK_RIGHT: return mir::Key::Right;
            case SDLK_TAB: return mir::Key::Tab;
            case SDLK_LSHIFT: return mir::Key::Lshift;
            case SDLK_LCTRL: return mir::Key::Lctrl;
            case SDLK_LALT: return mir::Key::Lalt;
            case SDLK_RSHIFT: return mir::Key::Rshift;
            case SDLK_RCTRL: return mir::Key::Rctrl;
            case SDLK_RALT: return mir::Key::Ralt;
            case SDLK_Q: return mir::Key::Q;
            case SDLK_W: return mir::Key::W;
            case SDLK_E: return mir::Key::E;
            case SDLK_R: return mir::Key::R;
            case SDLK_T: return mir::Key::T;
            case SDLK_Y: return mir::Key::Y;
            case SDLK_U: return mir::Key::U;
            case SDLK_I: return mir::Key::I;
            case SDLK_O: return mir::Key::O;
            case SDLK_P: return mir::Key::P;
            case SDLK_A: return mir::Key::A;
            case SDLK_S: return mir::Key::S;
            case SDLK_D: return mir::Key::D;
            case SDLK_F: return mir::Key::F;
            case SDLK_G: return mir::Key::G;
            case SDLK_H: return mir::Key::H;
            case SDLK_J: return mir::Key::J;
            case SDLK_K: return mir::Key::K;
            case SDLK_L: return mir::Key::L;
            case SDLK_Z: return mir::Key::Z;
            case SDLK_X: return mir::Key::X;
            case SDLK_C: return mir::Key::C;
            case SDLK_V: return mir::Key::V;
            case SDLK_B: return mir::Key::B;
            case SDLK_N: return mir::Key::N;
            case SDLK_M: return mir::Key::M;
            case SDLK_F1: return mir::Key::F1;
            case SDLK_F2: return mir::Key::F2;
            case SDLK_F3: return mir::Key::F3;
            case SDLK_F4: return mir::Key::F4;
            case SDLK_F5: return mir::Key::F5;
            case SDLK_F6: return mir::Key::F6;
            case SDLK_F7: return mir::Key::F7;
            case SDLK_F8: return mir::Key::F8;
            case SDLK_F9: return mir::Key::F9;
            case SDLK_F10: return mir::Key::F10;
            case SDLK_F11: return mir::Key::F11;
            case SDLK_F12: return mir::Key::F12;
            default: return mir::Key::COUNT;
        }
    }
}

namespace mir::border {
    float borderR = 255, borderG = 255, borderB = 255, borderA = 255;
    float borderX = 0, borderY = 0;
    float borderW = 0, borderH = 0;

    void SetColor(const float r, const float g, const float b, const float a) noexcept {
        borderR = r; borderG = g; borderB = b; borderA = a;
    }
    void SetPosition(const float x, const float y) noexcept {
        borderX = x; borderY = y;
    }
    void SetSize(const float w, const float h) noexcept {
        borderW = w; borderH = h;
    }
}

namespace mir::label {
    String<> labelText = "";
    String<> labelFont = "";
    float labelSize = 24;
    float labelR = 255, labelG = 255, labelB = 255, labelA = 255;
    float labelX = 0, labelY = 0;

    void SetText(const String<>& text) noexcept { labelText = text; }
    void SetFont(const String<>& font) noexcept { labelFont = font; }
    void SetSize(const float size) noexcept { labelSize = size; }
    void SetColor(const float r, const float g, const float b, const float a) noexcept {
        labelR = r; labelG = g; labelB = b; labelA = a;
    }
    void SetPosition(const float x, const float y) noexcept {
        labelX = x; labelY = y;
    }
}

namespace mir::button {
    String<> buttonText = "";
    String<> buttonFont = "";
    float bgR = 128, bgG = 128, bgB = 128, bgA = 255;
    float fgR = 255, fgG = 255, fgB = 255, fgA = 255;
    float buttonX = 0, buttonY = 0;
    float buttonW = 0, buttonH = 0;

    void SetText(const String<>& text) noexcept { buttonText = text; }
    void SetFont(const String<>& font) noexcept { buttonFont = font; }
    void SetBackgroundColor(const float r, const float g, const float b, const float a) noexcept {
        bgR = r; bgG = g; bgB = b; bgA = a;
    }
    void SetForegroundColor(const float r, const float g, const float b, const float a) noexcept {
        fgR = r; fgG = g; fgB = b; fgA = a;
    }
    void SetPosition(const float x, const float y) noexcept {
        buttonX = x; buttonY = y;
    }
    void SetSize(const float w, const float h) noexcept {
        buttonW = w; buttonH = h;
    }
}

namespace mir::window {
    PointerHandle<void> GetWindow() noexcept {
        return PointerHandle<void>();
    }

    bool IsOpen() noexcept {
        return sdl::isWindowOpen;
    }

    bool IsOpening() noexcept {
        return sdl::isWindowOpen;
    }

    void Init(
        const String<>& title,
        Mode mode,
        Resolution res,
        uint32_t width,
        uint32_t height) noexcept {
        
        if (sdl::isWindowOpen) return;

        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
        TTF_Init();

        Title = title;
        ScreenWidth = 1920;
        ScreenHeight = 1080;

        switch (res) {
            case Resolution::HD: ScreenWidth = 1280; ScreenHeight = 720; break;
            case Resolution::FHD: ScreenWidth = 1920; ScreenHeight = 1080; break;
            case Resolution::QHD: ScreenWidth = 2560; ScreenHeight = 1440; break;
            case Resolution::UHD: ScreenWidth = 3840; ScreenHeight = 2160; break;
            case Resolution::Custom: ScreenWidth = width; ScreenHeight = height; break;
        }

        SDL_WindowFlags flags = 0;
        switch (mode) {
            case Mode::Windowed: break;
            case Mode::Fullscreen: flags |= SDL_WINDOW_FULLSCREEN; break;
            case Mode::Borderless: flags |= SDL_WINDOW_BORDERLESS; break;
            case Mode::Desktop: flags |= SDL_WINDOW_FULLSCREEN; break;
        }

        sdl::sdlWindow = SDL_CreateWindow(title.CStr(), ScreenWidth, ScreenHeight, flags);
        if (sdl::sdlWindow) {
            sdl::sdlRenderer = SDL_CreateRenderer(sdl::sdlWindow, nullptr);
            sdl::isWindowOpen = true;
            sdl::lastFrameTime = SDL_GetTicks();

            MIX_Init();
            sdl::sdlMixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
        }
    }

    void ProcessEvents() noexcept {
        mir::input::Update();
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                sdl::isWindowOpen = false;
            }
            else if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
                bool pressed = (event.type == SDL_EVENT_KEY_DOWN);
                mir::Key key = sdl::MapSDLKey(event.key.key);
                mir::input::SetKeyState(key, pressed);
            }
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
                bool pressed = (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
                std::size_t button = 0;
                if (event.button.button == SDL_BUTTON_LEFT) button = 0;
                else if (event.button.button == SDL_BUTTON_MIDDLE) button = 1;
                else if (event.button.button == SDL_BUTTON_RIGHT) button = 2;
                mir::input::SetMouseState(button, pressed);
            }
            else if (event.type == SDL_EVENT_MOUSE_MOTION) {
                mir::input::SetMousePosition(event.motion.x, event.motion.y);
            }
        }
    }

    static inline void DrawUI() {
        if (border::borderW > 0 && border::borderH > 0) {
            SDL_SetRenderDrawColor(sdl::sdlRenderer, border::borderR, border::borderG, border::borderB, border::borderA);
            SDL_FRect rect = { border::borderX, border::borderY, border::borderW, border::borderH };
            SDL_RenderRect(sdl::sdlRenderer, &rect);
        }

        if (button::buttonW > 0 && button::buttonH > 0) {
            SDL_SetRenderDrawColor(sdl::sdlRenderer, button::bgR, button::bgG, button::bgB, button::bgA);
            SDL_FRect rect = { button::buttonX, button::buttonY, button::buttonW, button::buttonH };
            SDL_RenderFillRect(sdl::sdlRenderer, &rect);

            if (!button::buttonText.Empty()) {
                sdl::DrawTextSDL(
                    button::buttonText, button::buttonFont, 
                    button::buttonX + 5, button::buttonY + 5, 20, 
                    button::fgR, button::fgG, button::fgB, button::fgA);
            }
        }

        if (!label::labelText.Empty()) {
            sdl::DrawTextSDL(
                label::labelText, label::labelFont, 
                label::labelX, label::labelY, label::labelSize, 
                label::labelR, label::labelG, label::labelB, label::labelA);
        }
    }

    void Render() noexcept {
        if (!sdl::sdlRenderer) return;

        // Draw Sprites
        for (std::size_t i = 1; i < MAX_ID; ++i) {
            Id id(i, 0);
            if (!mir::core::Manager::Instance().IsValidEntity(id)) continue;
            if (!mir::sprite::Texture::IsValidEntity(id)) continue;

            const String<>& textureName = mir::sprite::Texture::Get(id);
            SDL_Texture* tex = sdl::GetLoadedTexture(textureName);
            if (!tex) continue;

            float px = mir::transform::PositionX::IsValidEntity(id) ? mir::transform::PositionX::Get(id) : 0.f;
            float py = mir::transform::PositionY::IsValidEntity(id) ? mir::transform::PositionY::Get(id) : 0.f;
            float rot = mir::transform::Rotation::IsValidEntity(id) ? mir::transform::Rotation::Get(id) : 0.f;
            float scale = mir::transform::Scale::IsValidEntity(id) ? mir::transform::Scale::Get(id) : 1.f;

            float sw = mir::sprite::SourceWidth::IsValidEntity(id) ? mir::sprite::SourceWidth::Get(id) : 0.f;
            float sh = mir::sprite::SourceHeight::IsValidEntity(id) ? mir::sprite::SourceHeight::Get(id) : 0.f;
            float dw = mir::sprite::DestinationWidth::IsValidEntity(id) ? mir::sprite::DestinationWidth::Get(id) : 0.f;
            float dh = mir::sprite::DestinationHeight::IsValidEntity(id) ? mir::sprite::DestinationHeight::Get(id) : 0.f;
            float ax = mir::sprite::AnchorX::IsValidEntity(id) ? mir::sprite::AnchorX::Get(id) : 0.5f;
            float ay = mir::sprite::AnchorY::IsValidEntity(id) ? mir::sprite::AnchorY::Get(id) : 0.5f;

            if (sw <= 0.f || sh <= 0.f) {
                float texW = 0.f, texH = 0.f;
                SDL_GetTextureSize(tex, &texW, &texH);
                if (sw <= 0.f) sw = texW;
                if (sh <= 0.f) sh = texH;
            }
            if (dw <= 0.f) dw = sw;
            if (dh <= 0.f) dh = sh;

            dw *= scale;
            dh *= scale;

            SDL_FRect dstRect = { px - dw * ax, py - dh * ay, dw, dh };
            SDL_FRect srcRect = { 0.f, 0.f, sw, sh };

            std::uint8_t alpha = mir::sprite::Alpha::IsValidEntity(id) ? mir::sprite::Alpha::Get(id) : 255;
            std::uint8_t r = mir::sprite::TintRed::IsValidEntity(id) ? mir::sprite::TintRed::Get(id) : 255;
            std::uint8_t g = mir::sprite::TintGreen::IsValidEntity(id) ? mir::sprite::TintGreen::Get(id) : 255;
            std::uint8_t b = mir::sprite::TintBlue::IsValidEntity(id) ? mir::sprite::TintBlue::Get(id) : 255;

            SDL_SetTextureColorMod(tex, r, g, b);
            SDL_SetTextureAlphaMod(tex, alpha);

            SDL_FPoint center = { dw * ax, dh * ay };
            SDL_RenderTextureRotated(sdl::sdlRenderer, tex, &srcRect, &dstRect, rot, &center, SDL_FLIP_NONE);
        }

        DrawUI();
    }

    void Display() noexcept {
        if (!sdl::sdlRenderer) return;
        SDL_RenderPresent(sdl::sdlRenderer);

        // Frame cap delay
        std::uint64_t now = SDL_GetTicks();
        std::uint64_t elapsed = now - sdl::lastFrameTime;
        if (elapsed < sdl::frameDelay) {
            SDL_Delay(sdl::frameDelay - elapsed);
        }
        sdl::lastFrameTime = SDL_GetTicks();
    }

    void Clear(std::uint8_t red, std::uint8_t green, std::uint8_t blue) noexcept {
        if (!sdl::sdlRenderer) return;
        SDL_SetRenderDrawColor(sdl::sdlRenderer, red, green, blue, 255);
        SDL_RenderClear(sdl::sdlRenderer);
    }

    void Close() noexcept {
        sdl::isWindowOpen = false;
    }

    void Shutdown() noexcept {
        if (sdl::sdlMixer) {
            MIX_DestroyMixer(sdl::sdlMixer);
            sdl::sdlMixer = nullptr;
            MIX_Quit();
        }
        if (sdl::sdlRenderer) {
            SDL_DestroyRenderer(sdl::sdlRenderer);
            sdl::sdlRenderer = nullptr;
        }
        if (sdl::sdlWindow) {
            SDL_DestroyWindow(sdl::sdlWindow);
            sdl::sdlWindow = nullptr;
        }
        TTF_Quit();
        SDL_Quit();
        sdl::isWindowOpen = false;
    }

    void SetTitle(const String<>& title) noexcept {
        Title = title;
        if (sdl::sdlWindow) {
            SDL_SetWindowTitle(sdl::sdlWindow, title.CStr());
        }
    }

    void SetFPS(const std::uint16_t fps) noexcept {
        sdl::fpsLimit = fps;
        sdl::frameDelay = 1000 / fps;
    }

    void SetSize(const std::uint16_t width, const std::uint16_t height) noexcept {
        ScreenWidth = width;
        ScreenHeight = height;
        if (sdl::sdlWindow) {
            SDL_SetWindowSize(sdl::sdlWindow, width, height);
        }
    }

    void SetMode(const Mode mode) noexcept {
        if (!sdl::sdlWindow) return;
        switch (mode) {
            case Mode::Windowed: SDL_SetWindowFullscreen(sdl::sdlWindow, false); break;
            case Mode::Fullscreen: SDL_SetWindowFullscreen(sdl::sdlWindow, true); break;
            case Mode::Borderless: SDL_SetWindowBordered(sdl::sdlWindow, false); break;
            case Mode::Desktop: SDL_SetWindowFullscreen(sdl::sdlWindow, true); break;
        }
    }

    void SetResolution(const Resolution resolution) noexcept {
        std::uint16_t w = 1920;
        std::uint16_t h = 1080;
        switch (resolution) {
            case Resolution::HD: w = 1280; h = 720; break;
            case Resolution::FHD: w = 1920; h = 1080; break;
            case Resolution::QHD: w = 2560; h = 1440; break;
            case Resolution::UHD: w = 3840; h = 2160; break;
            default: return;
        }
        SetSize(w, h);
    }

    Point2<float> MapPixelToCoords(const Point2<int>& pixel) noexcept {
        return Point2<float>(static_cast<float>(pixel.x), static_cast<float>(pixel.y));
    }
}
