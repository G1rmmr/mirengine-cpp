#include "math/Math.hpp"
#include "core/Manager.hpp"
#include "core/Entity.hpp"
#include "component/Transform.hpp"
#include "component/Sprite.hpp"
#include "script/ScriptSystem.hpp"
#include <iostream>
#include <cassert>
#include <exception>

using namespace mir;

void TestMath() {
    std::cout << "Running Math Tests..." << std::endl;
    
    // Vector2 tests
    math::Vector2 v1(1.0f, 2.0f);
    math::Vector2 v2(3.0f, 4.0f);
    math::Vector2 v3 = v1 + v2;
    assert(v3.X == 4.0f && v3.Y == 6.0f);
    
    // Lerp test
    float l = math::Lerp(0.f, 10.f, 0.5f);
    assert(l == 5.0f);
    
    std::cout << "Math Tests Passed!" << std::endl;
}

void TestCore() {
    std::cout << "Running Core Tests..." << std::endl;
    
    auto& manager = core::Manager::Instance();
    Id entity = manager.AddEntity();
    assert(manager.IsValidEntity(entity));
    
    // Add component
    transform::PositionX::Set(entity, 42.0f);
    manager.UpdateSystem(0.0f); // commit command buffer
    
    assert(transform::PositionX::IsValidEntity(entity));
    assert(transform::PositionX::Get(entity) == 42.0f);
    
    // Delete entity
    manager.DeleteEntity(entity);
    manager.UpdateSystem(0.0f); // commit delete
    
    assert(!manager.IsValidEntity(entity));
    
    std::cout << "Core Tests Passed!" << std::endl;
}

void TestScript() {
    std::cout << "Running Script Tests..." << std::endl;
    
    auto& scriptSys = script::ScriptSystem::Instance();
    scriptSys.Initialize();
    
    sol::state& lua = scriptSys.GetLuaState();
    
    auto result = lua.safe_script(R"(
        local manager = Manager.Instance()
        local entity = manager:AddEntity()
        Transform.setPosition(entity, 100.0, 200.0)
        Sprite.setTexture(entity, "assets/hero.png")
        return entity
    )");
    
    assert(result.valid());
    mir::Id entity = result;
    
    // Commit the command buffer in C++ side to apply component updates
    core::Manager::Instance().UpdateSystem(0.0f);
    
    assert(transform::PositionX::Get(entity) == 100.0f);
    assert(transform::PositionY::Get(entity) == 200.0f);
    assert(sprite::Texture::Get(entity) == "assets/hero.png");
    
    scriptSys.Shutdown();
    std::cout << "Script Tests Passed!" << std::endl;
}

int main() {
    try {
        TestMath();
        TestCore();
        TestScript();
        std::cout << "All tests passed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
