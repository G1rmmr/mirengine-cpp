#include "device/Window.hpp"
#include "script/ScriptSystem.hpp"
#include "core/Manager.hpp"
#include <SDL3/SDL.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <sol/sol.hpp>
#include <cctype>

using namespace mir;

static inline std::string trim(std::string s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
    return s;
}

struct RuntimeConfig {
    std::string title = "MIR Engine";
    window::Mode mode = window::Mode::Windowed;
    window::Resolution resolution = window::Resolution::HD;
    uint32_t width = 1280;
    uint32_t height = 720;
};

RuntimeConfig LoadRuntimeConfig() {
    RuntimeConfig config;
    sol::state lua;
    
    // config.lua 파일 실행 전, 열거형(enum) 바인딩
    lua.new_enum<window::Mode>("WindowMode", {
        {"Windowed", window::Mode::Windowed},
        {"Fullscreen", window::Mode::Fullscreen},
        {"Borderless", window::Mode::Borderless},
        {"Desktop", window::Mode::Desktop}
    });

    lua.new_enum<window::Resolution>("WindowResolution", {
        {"HD", window::Resolution::HD},
        {"FHD", window::Resolution::FHD},
        {"QHD", window::Resolution::QHD},
        {"UHD", window::Resolution::UHD},
        {"Custom", window::Resolution::Custom}
    });

    // config.lua 파일 실행 (오류 시 무시하고 기본값 반환)
    auto result = lua.safe_script_file("config.lua", sol::script_pass_on_error);
    if (!result.valid()) {
        std::cerr << "config.lua not found or has errors. Using default settings." << std::endl;
        return config;
    }

    config.title = lua["WINDOW_TITLE"].get_or<std::string>("MIR Engine");
    config.mode = lua["WINDOW_MODE"].get_or(window::Mode::Windowed);
    config.resolution = lua["WINDOW_RESOLUTION"].get_or(window::Resolution::HD);
    config.width = lua["WINDOW_WIDTH"].get_or<uint32_t>(1280);
    config.height = lua["WINDOW_HEIGHT"].get_or<uint32_t>(720);

    return config;
}

int main(int argc, char* argv[]) {
    // 1. 윈도우 초기화 (config.lua가 존재하면 설정을 읽어서 적용)
    RuntimeConfig config = LoadRuntimeConfig();
    window::Init(
        String<>(config.title.c_str()),
        config.mode,
        config.resolution,
        config.width,
        config.height
    );
    
    if (!window::IsOpen()) {
        std::cerr << "Failed to initialize game window." << std::endl;
        return -1;
    }

    // 2. 스크립트 시스템 초기화 (내부적으로 script/main.lua 로딩 및 Init() 호출)
    auto& scriptSys = script::ScriptSystem::Instance();
    scriptSys.Initialize();
    
    // Commit entities and components created during Init()
    core::Manager::Instance().UpdateSystem(0.0f);

    std::cout << "MIR Engine Started Successfully!" << std::endl;

    std::uint64_t lastTime = SDL_GetTicks();

    // 3. 메인 게임 루프
    while (window::IsOpen()) {
        // 이벤트 처리 및 입력 업데이트
        window::ProcessEvents();

        // 델타 타임(초 단위) 계산
        std::uint64_t currentTime = SDL_GetTicks();
        float deltaTime = static_cast<float>(currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        // Lua 스크립트 Update(deltaTime) 실행
        scriptSys.Update(deltaTime);

        // ECS 매니저의 시스템 업데이트 (명령 버퍼 반영 등)
        core::Manager::Instance().UpdateSystem(deltaTime);

        // 화면 지우기 (배경색: 검은색)
        window::Clear(0, 0, 0);

        // 등록된 Sprite 등 렌더링
        window::Render();

        // 버퍼 플립 및 프레임 레이트 동기화 (기본 60FPS)
        window::Display();
    }

    // 4. 리소스 및 시스템 정리
    scriptSys.Shutdown();
    window::Shutdown();

    std::cout << "MIR Engine Stopped." << std::endl;
    return 0;
}
