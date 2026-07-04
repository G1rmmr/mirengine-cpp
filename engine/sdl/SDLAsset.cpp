#include "SDLInternal.hpp"
#include "asset/Texture.hpp"
#include "asset/Font.hpp"
#include "asset/Sound.hpp"
#include "asset/Video.hpp"
#include "asset/Animation.hpp"
#include "asset/Resource.hpp"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <container/Map.hpp>
#include <container/List.hpp>
#include <string>

using namespace zet;

namespace mir::sdl {
    // Cache for assets
    static Map<String<>, SDL_Texture*, resource::MAX_RESOURCE> textureCache;
    static Map<String<>, TTF_Font*, resource::MAX_RESOURCE> fontCache;
    static Map<String<>, MIX_Audio*, resource::MAX_RESOURCE> soundCache;
    MIX_Track* bgmTrack = nullptr;
    MIX_Audio* currentBgm = nullptr;

    SDL_Texture* GetLoadedTexture(const String<>& name) noexcept {
        SDL_Texture** ptr = textureCache.Find(name);
        return ptr ? *ptr : nullptr;
    }

    void DrawTextSDL(
        const String<>& text, 
        const String<>& fontName, 
        float x, float y, float size, 
        std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a) noexcept {
        
        if (!sdlRenderer) return;

        // Font key: fontName_size
        std::string fontKeyStr = std::string(fontName.CStr()) + "_" + std::to_string(static_cast<int>(size));
        String<> key(fontKeyStr.c_str());

        TTF_Font* font = nullptr;
        TTF_Font** fontPtr = fontCache.Find(key);
        if (fontPtr) {
            font = *fontPtr;
        } else {
            String<> path = resource::GetPath(fontName);
            if (path.Empty()) {
                path = fontName;
            }
            font = TTF_OpenFont(path.CStr(), size);
            if (!font) return;
            fontCache.Insert(key, font);
        }

        SDL_Color color = { r, g, b, a };
        SDL_Surface* surf = TTF_RenderText_Blended(font, text.CStr(), text.Size(), color);
        if (!surf) return;

        SDL_Texture* tex = SDL_CreateTextureFromSurface(sdlRenderer, surf);
        SDL_DestroySurface(surf);
        if (!tex) return;

        float w = 0.f, h = 0.f;
        SDL_GetTextureSize(tex, &w, &h);
        SDL_FRect dstRect = { x, y, w, h };
        SDL_RenderTexture(sdlRenderer, tex, nullptr, &dstRect);
        SDL_DestroyTexture(tex);
    }
}

namespace mir::texture {
    void Load(const String<>& name) noexcept {
        if (sdl::GetLoadedTexture(name)) return;

        String<> path = resource::GetPath(name);
        if (path.Empty()) {
            path = name;
        }

        if (!sdl::sdlRenderer) return;

        SDL_Texture* tex = IMG_LoadTexture(sdl::sdlRenderer, path.CStr());
        if (tex) {
            sdl::textureCache.Insert(name, tex);
        }
    }
}

namespace mir::font {
    void Load(const String<>& name) noexcept {
        // Font loaded on demand in DrawTextSDL to support sizes
    }
}

namespace mir::sound {
    void Load(const String<>& name) noexcept {
        MIX_Audio** audioPtr = sdl::soundCache.Find(name);
        if (audioPtr) return;

        String<> path = resource::GetPath(name);
        if (path.Empty()) {
            path = name;
        }

        // Initialize audio device on demand if not done
        static bool audioOpened = false;
        if (!audioOpened) {
            MIX_Init();
            sdl::sdlMixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
            audioOpened = true;
        }

        if (!sdl::sdlMixer) return;

        MIX_Audio* audio = MIX_LoadAudio(sdl::sdlMixer, path.CStr(), true);
        if (audio) {
            sdl::soundCache.Insert(name, audio);
        }
    }

    void Play(const String<>& name, float volume, float pitch) noexcept {
        MIX_Audio** audioPtr = sdl::soundCache.Find(name);
        if (!audioPtr) {
            Load(name);
            audioPtr = sdl::soundCache.Find(name);
            if (!audioPtr) return;
        }

        if (sdl::sdlMixer) {
            MIX_Audio* audio = *audioPtr;
            // Play using a quick track
            MIX_Track* track = MIX_CreateTrack(sdl::sdlMixer);
            if (track) {
                MIX_SetTrackAudio(track, audio);
                MIX_SetTrackGain(track, volume / 100.f);
                MIX_PlayTrack(track, 0);
                MIX_DestroyTrack(track);
            }
            MIX_PlayAudio(sdl::sdlMixer, audio);
        }
    }

    void PlayAt(const String<>& name, const float x, const float y, float volume, float pitch) noexcept {
        Play(name, volume, pitch);
    }

    void PlayBgm(const String<>& name, float volume, bool loop) noexcept {
        String<> path = resource::GetPath(name);
        if (path.Empty()) {
            path = name;
        }

        static bool audioOpened = false;
        if (!audioOpened) {
            MIX_Init();
            sdl::sdlMixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
            audioOpened = true;
        }

        if (!sdl::sdlMixer) return;

        if (sdl::currentBgm) {
            if (sdl::bgmTrack) {
                MIX_StopTrack(sdl::bgmTrack, 0);
            }
            MIX_DestroyAudio(sdl::currentBgm);
            sdl::currentBgm = nullptr;
        }

        sdl::currentBgm = MIX_LoadAudio(sdl::sdlMixer, path.CStr(), true);
        if (sdl::currentBgm) {
            if (!sdl::bgmTrack) {
                sdl::bgmTrack = MIX_CreateTrack(sdl::sdlMixer);
            }
            if (sdl::bgmTrack) {
                MIX_SetTrackAudio(sdl::bgmTrack, sdl::currentBgm);
                MIX_SetTrackLoops(sdl::bgmTrack, loop ? -1 : 0);
                MIX_SetTrackGain(sdl::bgmTrack, volume / 100.f);
                MIX_PlayTrack(sdl::bgmTrack, 0);
            }
        }
    }

    void StopBgm() noexcept {
        if (sdl::bgmTrack) {
            MIX_StopTrack(sdl::bgmTrack, 0);
        }
    }

    void SetBgmVolume(float volume) noexcept {
        if (sdl::bgmTrack) {
            MIX_SetTrackGain(sdl::bgmTrack, volume / 100.f);
        }
    }

    void SetMasterVolume(float volume) noexcept {
        if (sdl::bgmTrack) {
            MIX_SetTrackGain(sdl::bgmTrack, volume / 100.f);
        }
    }

    void StopAll() noexcept {
        if (sdl::sdlMixer) {
            MIX_StopAllTracks(sdl::sdlMixer, 0);
        }
    }
}

namespace mir::video {
    void Load(const String<>& name) noexcept {}
    void Play(const String<>& name, float speed) noexcept {}
    void Stop() noexcept {}
    void SetVideoTimeline(const float timeStamp) noexcept {}
    void StopAll() noexcept {}
}

namespace mir::animation {
    struct AnimDef {
        List<Frame> frames;
    };
    static Map<String<>, AnimDef, resource::MAX_RESOURCE> anims;

    void Register(const String<>& name, const List<Frame>& frames) noexcept {
        anims.Insert(name, AnimDef{ frames });
    }

    void Play(const Id id, const String<>& animName, float speed, bool loop) noexcept {}
    void Stop(const Id id) noexcept {}
}
