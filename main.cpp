#include "device/Window.hpp"
#include "system/ScriptSystem.hpp"
#include "util/Timer.hpp"
#include <exception>

int main() {
    try {
        mir::window::Init("Mirengine", mir::window::Mode::Windowed, mir::window::Resolution::HD, 1280, 720);
        mir::system::ScriptSystem::Instance().Initialize();

        while (mir::window::IsOpening()) {
            const float deltaTime = mir::time::GetDelta();
            
            mir::window::ProcessEvents();

            mir::system::ScriptSystem::Instance().Update(deltaTime);

            mir::window::Clear(0, 0, 0);
            mir::window::Render();
            mir::window::Display();
        }
        
        mir::system::ScriptSystem::Instance().Shutdown();
        mir::window::Shutdown();
    }
    catch (const std::exception& e) {
        mir::system::ScriptSystem::Instance().Shutdown();
        mir::window::Shutdown();
    }
    return 0;
}
