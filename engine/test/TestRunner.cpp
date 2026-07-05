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
        Transform.SetPosition(entity, 100.0, 200.0)
        Sprite.SetTexture(entity, "assets/hero.png")
        return entity
    )");
    
    assert(result.valid());
    mir::Id entity = result;
    
    // Commit the command buffer in C++ side to apply component updates
    core::Manager::Instance().UpdateSystem(0.0f);
    
    assert(transform::PositionX::Get(entity) == 100.0f);
    assert(transform::PositionY::Get(entity) == 200.0f);
    assert(sprite::Texture::Get(entity) == "assets/hero.png");
    
    // Test 1: Texture.Load failure returns false
    auto result2 = lua.safe_script(R"(
        return Texture.Load("nonexistent_file_abc123.png")
    )");
    assert(result2.valid() && result2.get<bool>() == false);
    
    // Test 2: Collision.Update safety with missing components
    auto result3 = lua.safe_script(R"(
        local manager = Manager.Instance()
        local e1 = manager:AddEntity()
        local e2 = manager:AddEntity()
        return Collision.Update(e1, e2)
    )");
    assert(result3.valid() && result3.get<bool>() == false);

    // Test 3: Collision.Update functionality
    auto result4 = lua.safe_script(R"(
        local manager = Manager.Instance()
        local e1 = manager:AddEntity()
        local e2 = manager:AddEntity()
        
        Transform.SetPosition(e1, 0.0, 0.0)
        Collider.SetBound(e1, 10.0, 10.0)
        Collider.SetShouldTrigger(e1, true)
        
        Transform.SetPosition(e2, 5.0, 5.0)
        Collider.SetBound(e2, 10.0, 10.0)
        Collider.SetShouldTrigger(e2, true)
        
        return {e1, e2}
    )");
    assert(result4.valid());
    sol::table entities = result4;
    mir::Id e1 = entities[1];
    mir::Id e2 = entities[2];
    
    // Commit component creation
    core::Manager::Instance().UpdateSystem(0.0f);
    
    // Verify they collide
    auto result5 = lua.safe_script(R"(
        local e1, e2 = ...
        return Collision.Update(e1, e2)
    )", e1, e2);
    assert(result5.valid() && result5.get<bool>() == true);
    
    // Move e2 out of collision range
    lua.safe_script(R"(
        local e1, e2 = ...
        Transform.SetPosition(e2, 20.0, 20.0)
    )", e1, e2);
    core::Manager::Instance().UpdateSystem(0.0f);
    
    // Verify they do not collide anymore
    auto result6 = lua.safe_script(R"(
        local e1, e2 = ...
        return Collision.Update(e1, e2)
    )", e1, e2);
    assert(result6.valid() && result6.get<bool>() == false);
    
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
