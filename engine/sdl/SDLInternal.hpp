#pragma once

#include <SDL3/SDL.h>
#include <cstdint>
#include <container/String.hpp>

struct MIX_Mixer;

using namespace zet;

namespace mir::sdl {
    extern SDL_Window* sdlWindow;
    extern SDL_Renderer* sdlRenderer;
    extern MIX_Mixer* sdlMixer;

    SDL_Texture* GetLoadedTexture(const String<>& name) noexcept;
    void DrawTextSDL(
        const String<>& text, 
        const String<>& fontName, 
        float x, float y, float size, 
        std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a) noexcept;
}
