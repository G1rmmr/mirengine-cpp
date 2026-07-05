#include "device/Window.hpp"
#include "script/ScriptSystem.hpp"
#include "core/Manager.hpp"
#include <SDL3/SDL.h>
#include <iostream>

using namespace mir;

int main(int argc, char* argv[]) {
    // 1. 윈도우 초기화 (HD 해상도: 1280x720, 창 모드)
    window::Init("MIR Engine", window::Mode::Windowed, window::Resolution::HD);
    
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
