#include "src/App.hpp"
#include "view/backend/SFMLDisplay.hpp"

int main() {
    try{
        app::Initialize();
        while(mir::window::IsOpening()){
            const mir::Real deltaTime = mir::time::GetDelta();
            app::ProcessInput();
            app::Update(deltaTime);
            app::Render();
        }
        app::Shutdown();
    }
    catch(const std::exception& e){
        mir::debug::Log("Exception: %s", e.what());
        app::Shutdown();
    }
    return 0;
}
