#include "device/Window.hpp"
#include "script/ScriptSystem.hpp"
#include "core/Manager.hpp"
#include <SDL3/SDL.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
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
    std::ifstream file("mirengine_config.txt");
    if (!file.is_open()) {
        return config;
    }
    std::string line;
    while (std::getline(file, line)) {
        auto commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        line = trim(line);
        if (line.empty()) continue;

        auto eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;

        std::string key = trim(line.substr(0, eqPos));
        std::string val = trim(line.substr(eqPos + 1));

        if (key == "WINDOW_TITLE") {
            config.title = val;
        } else if (key == "WINDOW_MODE") {
            if (val == "Windowed") config.mode = window::Mode::Windowed;
            else if (val == "Fullscreen") config.mode = window::Mode::Fullscreen;
            else if (val == "Borderless") config.mode = window::Mode::Borderless;
            else if (val == "Desktop") config.mode = window::Mode::Desktop;
        } else if (key == "WINDOW_RESOLUTION") {
            if (val == "HD") config.resolution = window::Resolution::HD;
            else if (val == "FHD") config.resolution = window::Resolution::FHD;
            else if (val == "QHD") config.resolution = window::Resolution::QHD;
            else if (val == "UHD") config.resolution = window::Resolution::UHD;
            else if (val == "Custom") config.resolution = window::Resolution::Custom;
        } else if (key == "WINDOW_WIDTH") {
            try { config.width = std::stoul(val); } catch (...) {}
        } else if (key == "WINDOW_HEIGHT") {
            try { config.height = std::stoul(val); } catch (...) {}
        }
    }
    return config;
}

int main(int argc, char* argv[]) {
    // 1. 윈도우 초기화 (mirengine_config.txt가 존재하면 설정을 읽어서 적용)
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
