#include "MIR.hpp"
#include "math/Math.hpp"
#include "core/Manager.hpp"
#include "core/Entity.hpp"
#include "component/Transform.hpp"
#include "component/Sprite.hpp"
#include "component/Rigidbody.hpp"
#include "system/Movement.hpp"
#include "system/Event.hpp"
#include "system/Hierarchy.hpp"
#include "asset/Scene.hpp"
#include "asset/Resource.hpp"
#include "asset/Animation.hpp"
#include "script/ScriptSystem.hpp"
#include "util/Timer.hpp"
#include <iostream>
#include <cassert>
#include <cmath>
#include <exception>
#include <stdexcept>

using namespace mir;

void RegisterResourceFromSeparateTranslationUnit();

namespace {
    int receivedEventCount = 0;
	bool captureSystemOrder = false;
	int systemOrder[3]{};
	std::size_t systemOrderCount = 0;
	int postCommitCount = 0;
	int timerCallbackCount = 0;

    void CountEvent(Id) {
        ++receivedEventCount;
    }

	void SystemA(float) {
		if (captureSystemOrder) systemOrder[systemOrderCount++] = 1;
	}

	void SystemB(float) {
		if (captureSystemOrder) systemOrder[systemOrderCount++] = 2;
	}

	void SystemC(float) {
		if (captureSystemOrder) systemOrder[systemOrderCount++] = 3;
	}

	void PostCommitSystem(float) {
		++postCommitCount;
	}

	void Require(const bool condition, const char* const message) {
		if (!condition) throw std::runtime_error(message);
	}
}

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

    // HorizonSum must not depend on SSE4.1 (_mm_dp_ps). Verify both input and
    // output masks so the portable SSE2 fallback keeps DPPS semantics.
    alignas(16) float sumLanes[4]{};
    simd::Store(sumLanes, simd::HorizonSum<0xF5>(simd::Set(1.f, 2.f, 3.f, 4.f), simd::Set(1.f)));
    assert(sumLanes[0] == 10.f && sumLanes[1] == 0.f && sumLanes[2] == 10.f && sumLanes[3] == 0.f);
    assert(math::Vector2(1.f, 2.f).Dot(math::Vector2(3.f, 4.f)) == 11.f);
    assert(math::Vector3(1.f, 2.f, 3.f).Dot(math::Vector3(4.f, 5.f, 6.f)) == 32.f);
    
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

	Id replacement = manager.AddEntity();
	if (replacement.Index != entity.Index || replacement.Generation == entity.Generation) {
		throw std::runtime_error("Entity slot reuse did not advance generation");
	}
	transform::PositionX::Set(replacement, 7.0f);
	manager.UpdateSystem(0.0f);
	if (transform::PositionX::TryGet(entity) != nullptr) {
		throw std::runtime_error("TryGet exposed a stale entity");
	}
	if (transform::PositionX::IsValidEntity(entity)) {
		throw std::runtime_error("Stale entity accessed replacement component");
	}
	transform::PositionX::Remove(entity);
	if (!transform::PositionX::IsValidEntity(replacement) || transform::PositionX::Get(replacement) != 7.0f) {
		throw std::runtime_error("Stale entity removed replacement component");
	}
    
    std::cout << "Core Tests Passed!" << std::endl;
}

void TestSystemGraph() {
	std::cout << "Running System Graph Tests..." << std::endl;
	auto& manager = core::Manager::Instance();

	// Register in reverse dependency order. The graph, not registration order,
	// must determine A -> B -> C execution.
	const core::SystemId systemC = manager.RegisterSystem(&SystemC);
	const core::SystemId systemA = manager.RegisterSystem(&SystemA);
	const core::SystemId systemB = manager.RegisterSystem(&SystemB);
	const core::SystemId postCommit = manager.RegisterSystem(&PostCommitSystem, core::SystemPhase::PostCommit);
	Require(manager.IsValidSystem(systemA), "Failed to register system A");
	Require(manager.IsValidSystem(systemB), "Failed to register system B");
	Require(manager.IsValidSystem(systemC), "Failed to register system C");
	Require(manager.IsValidSystem(postCommit), "Failed to register post-commit system");
	Require(manager.AddSystemDependency(systemA, systemB), "Failed to add A -> B dependency");
	Require(manager.AddSystemDependency(systemB, systemC), "Failed to add B -> C dependency");
	Require(!manager.AddSystemDependency(systemC, systemA), "System dependency cycle was accepted");

	systemOrderCount = 0;
	postCommitCount = 0;
	captureSystemOrder = true;
	manager.UpdateSystem(0.0f);
	captureSystemOrder = false;
	Require(systemOrderCount == 3, "Unexpected simulation system count");
	Require(systemOrder[0] == 1 && systemOrder[1] == 2 && systemOrder[2] == 3, "System graph order is not A -> B -> C");
	Require(postCommitCount == 1, "Post-commit system did not run");
	std::cout << "System Graph Tests Passed!" << std::endl;
}

void TestHierarchy() {
	std::cout << "Running Hierarchy Tests..." << std::endl;
	auto& manager = core::Manager::Instance();
	const Id parent = manager.AddEntity();
	const Id child = manager.AddEntity();
	const Id grandchild = manager.AddEntity();

	Require(transform::SetPosition(parent, 10.0f, 20.0f), "Failed to set parent local position");
	Require(transform::Rotation::Set(parent, 90.0f), "Failed to set parent local rotation");
	Require(transform::Scale::Set(parent, 2.0f), "Failed to set parent local scale");
	Require(transform::SetPosition(child, 5.0f, 0.0f), "Failed to set child local position");
	Require(transform::SetPosition(grandchild, 1.0f, 0.0f), "Failed to set grandchild local position");
	Require(hierarchy::SetParent(child, parent), "Failed to assign parent relation");
	Require(hierarchy::SetParent(grandchild, child), "Failed to assign grandchild relation");
	manager.UpdateSystem(0.0f);

	Require(hierarchy::ParentOf(child) == parent, "Child parent relation was not committed");
	Require(hierarchy::ParentOf(grandchild) == child, "Grandchild parent relation was not committed");
	Require(hierarchy::FirstChildOf(parent) == child, "Child traversal index was not built");
	Require(!hierarchy::SetParent(parent, grandchild), "Hierarchy cycle was accepted");

	if (transform::WorldPositionX::TryGet(child) == nullptr) {
		throw std::runtime_error("Hierarchy post-commit transform system did not populate child world data");
	}
	Require(std::abs(transform::WorldPositionX::Get(child) - 10.0f) < 0.001f, "Incorrect child world X");
	Require(std::abs(transform::WorldPositionY::Get(child) - 30.0f) < 0.001f, "Incorrect child world Y");
	Require(std::abs(transform::WorldRotation::Get(child) - 90.0f) < 0.001f, "Incorrect child world rotation");
	Require(std::abs(transform::WorldScale::Get(child) - 2.0f) < 0.001f, "Incorrect child world scale");
	Require(std::abs(transform::WorldPositionX::Get(grandchild) - 10.0f) < 0.001f, "Incorrect grandchild world X");
	Require(std::abs(transform::WorldPositionY::Get(grandchild) - 32.0f) < 0.001f, "Incorrect grandchild world Y");

	const Id detachedLeaf = manager.AddEntity();
	Require(transform::SetPosition(detachedLeaf, 2.0f, 0.0f), "Failed to set detached leaf local position");
	Require(hierarchy::SetParent(detachedLeaf, parent), "Failed to parent detached leaf");
	manager.UpdateSystem(0.0f);
	Require(std::abs(transform::WorldPositionX::Get(detachedLeaf) - 10.0f) < 0.001f, "Incorrect attached leaf world X");
	Require(std::abs(transform::WorldPositionY::Get(detachedLeaf) - 24.0f) < 0.001f, "Incorrect attached leaf world Y");
	Require(hierarchy::ClearParent(detachedLeaf), "Failed to clear detached leaf parent");
	manager.UpdateSystem(0.0f);
	Require(std::abs(transform::WorldPositionX::Get(detachedLeaf) - 2.0f) < 0.001f, "Detached leaf retained a stale world X");
	Require(std::abs(transform::WorldPositionY::Get(detachedLeaf) - 0.0f) < 0.001f, "Detached leaf retained a stale world Y");

	// Relation changes rebuild the ZET traversal index from ECS relation data,
	// so clear/reparent remains safe even with several siblings.
	Require(hierarchy::ClearParent(child), "Failed to clear parent relation");
	manager.UpdateSystem(0.0f);
	Require(hierarchy::ParentOf(child) == INVALID_ID, "Cleared parent relation still exists");
	Require(hierarchy::ParentOf(grandchild) == child, "Clearing parent detached the grandchild");
	Require(std::abs(transform::WorldPositionX::Get(child) - 5.0f) < 0.001f, "Incorrect detached child world X");
	Require(std::abs(transform::WorldPositionY::Get(child) - 0.0f) < 0.001f, "Incorrect detached child world Y");
	Require(hierarchy::SetParent(child, parent), "Failed to reparent child");
	manager.UpdateSystem(0.0f);
	Require(hierarchy::ParentOf(child) == parent, "Reparent relation was not committed");
	Require(std::abs(transform::WorldPositionX::Get(child) - 10.0f) < 0.001f, "Incorrect reparented child world X");
	Require(std::abs(transform::WorldPositionY::Get(child) - 30.0f) < 0.001f, "Incorrect reparented child world Y");

	// Deleting a parent never destroys unrelated ECS state: children are
	// detached to the internal world root and render from their local values.
	Require(manager.DeleteEntity(parent), "Failed to queue parent deletion");
	manager.UpdateSystem(0.0f);
	Require(!manager.IsValidEntity(parent), "Parent was not deleted");
	Require(manager.IsValidEntity(child), "Parent deletion cascaded unexpectedly");
	Require(hierarchy::ParentOf(child) == INVALID_ID, "Child was not orphaned after parent deletion");
	Require(std::abs(transform::WorldPositionX::Get(child) - 5.0f) < 0.001f, "Incorrect orphaned child world X");
	Require(std::abs(transform::WorldPositionY::Get(child) - 0.0f) < 0.001f, "Incorrect orphaned child world Y");
	std::cout << "Hierarchy Tests Passed!" << std::endl;
}

void TestTagsAndEvents() {
    std::cout << "Running Tag and Event Tests..." << std::endl;
    auto& manager = core::Manager::Instance();
    Id entity = manager.AddEntity();

    if (!tag::Set(entity, "Player")) {
        throw std::runtime_error("Failed to queue persistent tag");
    }
    manager.UpdateSystem(0.0f);
    assert(tag::Tag::IsValidEntity(entity));
    assert(tag::Tag::Get(entity) == "Player");

    receivedEventCount = 0;
    assert(event::On("spawn", &CountEvent));
    assert(event::Emit(entity, "spawn"));
	manager.UpdateSystem(0.0f);
    assert(receivedEventCount == 1);
    assert(tag::Tag::IsValidEntity(entity));
    assert(tag::Tag::Get(entity) == "Player");
    std::cout << "Tag and Event Tests Passed!" << std::endl;
}

void TestResourceRegistry() {
    std::cout << "Running Resource Registry Tests..." << std::endl;
    RegisterResourceFromSeparateTranslationUnit();
    assert(resource::GetPath("cross-tu-resource") == "assets/cross-tu-resource.png");
	Require(resource::Register("temporary", "assets/temporary.png"), "Failed to register resource");
	Require(resource::Unregister("temporary"), "Failed to unregister resource");
	Require(resource::GetPath("temporary").Empty(), "Removed resource is still available");
    std::cout << "Resource Registry Tests Passed!" << std::endl;
}

void TestTimerAndScene() {
	std::cout << "Running Timer and Scene Tests..." << std::endl;
	auto& manager = core::Manager::Instance();
	timerCallbackCount = 0;
	const time::TimerHandle handle = time::Register(0.05f, []() { ++timerCallbackCount; });
	Require(time::IsValid(handle), "Failed to register fixed-capacity timer");
	manager.UpdateSystem(0.05f);
	Require(timerCallbackCount == 1 && !time::IsValid(handle), "One-shot timer did not fire exactly once");

	int sceneCallbackCount = 0;
	Require(scene::Register("test-scene", [&sceneCallbackCount]() { ++sceneCallbackCount; }), "Failed to register scene");
	Require(scene::Load("test-scene"), "Failed to load registered scene");
	Require(sceneCallbackCount == 1, "Scene callback did not run");
	Require(scene::Unregister("test-scene"), "Failed to unregister scene");
	std::cout << "Timer and Scene Tests Passed!" << std::endl;
}

void TestScript() {
    std::cout << "Running Script Tests..." << std::endl;
    
	auto& scriptSys = script::ScriptSystem::Instance();
	if (!scriptSys.Initialize()) {
		throw std::runtime_error("ScriptSystem initialization failed");
	}
    
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

	// Hierarchy is intentionally script-facing through entity Ids only.
	auto hierarchyResult = lua.safe_script(R"(
		local manager = Manager.Instance()
		local parent = manager:AddEntity()
		local child = manager:AddEntity()
		Transform.SetPosition(parent, 4.0, 0.0)
		Transform.SetPosition(child, 3.0, 0.0)
		assert(Hierarchy.SetParent(child, parent))
		return { parent, child }
	)");
	Require(hierarchyResult.valid(), "Lua hierarchy setup failed");
	sol::table hierarchyEntities = hierarchyResult;
	const mir::Id hierarchyParent = hierarchyEntities[1];
	const mir::Id hierarchyChild = hierarchyEntities[2];
	core::Manager::Instance().UpdateSystem(0.0f);
	Require(hierarchy::ParentOf(hierarchyChild) == hierarchyParent, "Lua hierarchy relation was not committed");
	Require(std::abs(transform::WorldPositionX::Get(hierarchyChild) - 7.0f) < 0.001f, "Lua hierarchy world transform is incorrect");
    
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
    sol::protected_function func5 = lua.safe_script(R"(
        return function(e1, e2)
            return Collision.Update(e1, e2)
        end
    )");
    assert(func5.valid());
    auto result5 = func5(e1, e2);
    assert(result5.valid() && result5.get<bool>() == true);
    
    // Move e2 out of collision range
    sol::protected_function func_move = lua.safe_script(R"(
        return function(e1, e2)
            Transform.SetPosition(e2, 20.0, 20.0)
        end
    )");
    assert(func_move.valid());
    func_move(e1, e2);
    core::Manager::Instance().UpdateSystem(0.0f);
    
    // Verify they do not collide anymore
    sol::protected_function func6 = lua.safe_script(R"(
        return function(e1, e2)
            return Collision.Update(e1, e2)
        end
    )");
    assert(func6.valid());
    auto result6 = func6(e1, e2);
    assert(result6.valid() && result6.get<bool>() == false);

	// Newly exposed runtime APIs must execute through the same command and
	// system barriers that C++ callers use.
	auto extendedBindings = lua.safe_script(R"(
		assert(Key.F12 ~= nil)
		assert(MouseButton.Left ~= nil)
		assert(type(Input.IsMousePressed) == "function")
		assert(type(Window.SetTitle) == "function")
		assert(type(Border.SetSize) == "function")
		assert(type(Label.SetText) == "function")
		assert(type(Button.SetText) == "function")
		Camera.SetPosition(12.0, 34.0)
		assert(Camera.GetX() == 12.0 and Camera.GetY() == 34.0)
		local vec = Vector2.new(3.0, 4.0)
		assert(vec:Length() == 5.0)
		assert(Math.ToDegree(Math.ToRadian(90.0)) > 89.9)
		local matrix = Math.CreateTranslation2D(vec)
		assert(matrix:Get(2, 0) == 3.0)
		local queried = 0
		assert(Manager.Instance():ForEachEntity(function(id)
			assert(Manager.Instance():IsValidEntity(id))
			queried = queried + 1
		end))
		assert(queried > 0)

		assert(Resource.Register("lua-resource", "assets/lua-resource.png"))
		assert(Resource.GetPath("lua-resource") == "assets/lua-resource.png")
		assert(Resource.Unregister("lua-resource"))

		local sceneRuns = 0
		assert(Scene.Register("lua-scene", function() sceneRuns = sceneRuns + 1 end))
		assert(Scene.Load("lua-scene"))
		assert(sceneRuns == 1)

		local animationEntity = Manager.Instance():AddEntity()
		assert(Animation.Register("lua-animation", {
			AnimationFrame.new(0.0, 0.0, 8.0, 8.0),
			{8.0, 0.0, 8.0, 8.0}
		}))
		assert(Animation.Play(animationEntity, "lua-animation", 1.0, true))

		luaEventCount = 0
		assert(Event.On("lua-event", function(id)
			assert(Manager.Instance():IsValidEntity(id))
			luaEventCount = luaEventCount + 1
		end))
		assert(Event.Emit(animationEntity, "lua-event"))

		luaTimerCount = 0
		local timer = Timer.After(0.01, function() luaTimerCount = luaTimerCount + 1 end)
		assert(timer:is_valid())

		luaSystemOrder = {}
		local second = System.Register(function() table.insert(luaSystemOrder, 2) end)
		local first = System.Register(function() table.insert(luaSystemOrder, 1) end)
		assert(second:is_valid() and first:is_valid())
		assert(System.AddDependency(first, second))
		return animationEntity
	)");
	Require(extendedBindings.valid(), "Extended Lua bindings failed to register");
	const Id animationEntity = extendedBindings;
	core::Manager::Instance().UpdateSystem(0.01f);
	Require(sprite::SourceX::IsValidEntity(animationEntity), "Lua animation did not create a source rectangle");
	auto extendedResult = lua.safe_script(R"(
		assert(luaEventCount == 1)
		assert(luaTimerCount == 1)
		assert(#luaSystemOrder == 2 and luaSystemOrder[1] == 1 and luaSystemOrder[2] == 2)
		return true
	)");
	Require(extendedResult.valid() && extendedResult.get<bool>(), "Extended Lua callbacks did not run correctly");
    
    scriptSys.Shutdown();
    std::cout << "Script Tests Passed!" << std::endl;
}

void TestMovement() {
	std::cout << "Running Movement Tests..." << std::endl;
	auto& manager = core::Manager::Instance();
	Id entity = manager.AddEntity();
	transform::SetPosition(entity, 0.0f, 0.0f);
	rigidbody::SetVelocity(entity, 0.0f, 0.0f);
	rigidbody::Gravity::Set(entity, 100.0f);
	rigidbody::OnGround::Set(entity, false);
	manager.UpdateSystem(0.0f);

	movement::Update(entity, 0.5f);
	manager.UpdateSystem(0.0f);
	if (rigidbody::VelocityY::Get(entity) != 50.0f) {
		throw std::runtime_error("Gravity velocity was not persisted");
	}

	movement::Update(entity, 0.5f);
	manager.UpdateSystem(0.0f);
	if (rigidbody::VelocityY::Get(entity) != 100.0f) {
		throw std::runtime_error("Gravity velocity did not accumulate");
	}
	std::cout << "Movement Tests Passed!" << std::endl;
}

void TestSpriteSourceRect() {
    std::cout << "Running Sprite Source Rectangle Tests..." << std::endl;
    auto& manager = core::Manager::Instance();
    Id entity = manager.AddEntity();
    sprite::SetSourceRect(entity, 16.0f, 32.0f, 48.0f, 64.0f);
    manager.UpdateSystem(0.0f);

    assert(sprite::SourceX::Get(entity) == 16.0f);
    assert(sprite::SourceY::Get(entity) == 32.0f);
    assert(sprite::SourceWidth::Get(entity) == 48.0f);
    assert(sprite::SourceHeight::Get(entity) == 64.0f);
    std::cout << "Sprite Source Rectangle Tests Passed!" << std::endl;
}

void TestAnimation() {
    std::cout << "Running Animation Tests..." << std::endl;
    auto& manager = core::Manager::Instance();
    Id entity = manager.AddEntity();

    animation::Frames frames;
    frames.Push(animation::Frame{0.0f, 0.0f, 16.0f, 16.0f});
    frames.Push(animation::Frame{16.0f, 0.0f, 16.0f, 16.0f});
	Require(animation::Register("test-walk", frames), "Failed to register animation");
	Require(animation::Play(entity, "test-walk", 1.0f, true), "Failed to play animation");
    manager.UpdateSystem(0.0f);

    assert(sprite::SourceX::Get(entity) == 0.0f);
    assert(sprite::SourceY::Get(entity) == 0.0f);
    manager.UpdateSystem(0.1f);
    assert(sprite::SourceX::Get(entity) == 16.0f);
    assert(sprite::SourceY::Get(entity) == 0.0f);
    std::cout << "Animation Tests Passed!" << std::endl;
}

void TestShader() {
    std::cout << "Running Shader Bindings Tests..." << std::endl;
    
	auto& scriptSys = script::ScriptSystem::Instance();
	if (!scriptSys.Initialize()) {
		throw std::runtime_error("ScriptSystem initialization failed");
	}
    
    sol::state& lua = scriptSys.GetLuaState();
    
    // GPU Table validation
    auto result1 = lua.safe_script(R"(
        assert(GPU ~= nil)
        assert(type(GPU.CreateDevice) == "function")
		assert(GPUDevice ~= nil)
        assert(GPUShaderStage ~= nil)
        assert(GPUShaderStage.Vertex ~= nil)
        assert(GPU_SHADERFORMAT_SPIRV ~= nil)
        return true
    )");
    assert(result1.valid() && result1.get<bool>() == true);

    // Shader Class instantiation test
    auto result2 = lua.safe_script(R"(
        local vs = Shader.new()
        assert(vs ~= nil)
        assert(type(vs.LoadFromFile) == "function")
        vs:Destroy()
        return true
    )");
    assert(result2.valid() && result2.get<bool>() == true);

    // GPUPipeline Class instantiation test
    auto result3 = lua.safe_script(R"(
        local pipeline = GPUPipeline.new()
        assert(pipeline ~= nil)
        assert(type(pipeline.Create) == "function")
        pipeline:Destroy()
        return true
    )");
    assert(result3.valid() && result3.get<bool>() == true);

    scriptSys.Shutdown();
    std::cout << "Shader Bindings Tests Passed!" << std::endl;
}

int main() {
    try {
        TestMath();
		TestCore();
		TestSystemGraph();
		TestHierarchy();
		TestTagsAndEvents();
		TestResourceRegistry();
		TestTimerAndScene();
		TestMovement();
        TestSpriteSourceRect();
        TestAnimation();
        TestScript();
        TestShader();
        std::cout << "All tests passed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
