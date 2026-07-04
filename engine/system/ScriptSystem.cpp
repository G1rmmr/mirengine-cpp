#include "ScriptSystem.hpp"
#include "../core/Manager.hpp"
#include "../component/Transform.hpp"
#include "../device/Input.hpp"
#include "../device/Key.hpp"
#include "../util/Debugger.hpp"

#include <iostream>
#include <string>

namespace mir::system {

    void ScriptSystem::Initialize() {
        // Open default libraries
        lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::table, sol::lib::string, sol::lib::math);

        // Bind Debug Log override for print
        lua.set_function("print", [](sol::variadic_args va) {
            std::string logMsg = "";
            for (auto target : va) {
                // Convert each argument to string manually or via sol
                std::string argStr = target.as<std::string>();
                logMsg += argStr + " ";
            }
            if (!logMsg.empty()) {
                logMsg.pop_back(); // Remove trailing space
            }
            mir::debug::Log("[Lua] %s", logMsg.c_str());
        });

        // Bind Id type
        lua.new_usertype<mir::Id>("Id",
            sol::constructors<mir::Id(), mir::Id(const std::size_t, const std::size_t)>(),
            sol::meta_function::to_string, [](const mir::Id& id) {
                return "EntityId(" + std::to_string(static_cast<std::size_t>(id)) + ")";
            },
            "index", &mir::Id::Index,
            "generation", &mir::Id::Generation
        );

        // Bind Manager singleton
        auto manager_type = lua.new_usertype<mir::core::Manager>("Manager", sol::no_constructor);
        manager_type["Instance"] = &mir::core::Manager::Instance;
        manager_type["AddEntity"] = [](mir::core::Manager& m) -> mir::Id {
            return m.AddEntity();
        };
        manager_type["DeleteEntity"] = [](mir::core::Manager& m, const mir::Id id) {
            m.DeleteEntity(id);
        };
        manager_type["IsValidEntity"] = [](mir::core::Manager& m, const mir::Id id) -> bool {
            return m.IsValidEntity(id);
        };

        // Bind Transform namespace functions
        auto transform = lua.create_table();
        transform["setPosition"] = &mir::transform::SetPosition;
        transform["getPositionX"] = [](mir::Id id) -> float {
            return mir::transform::PositionX::IsValidEntity(id) ? mir::transform::PositionX::Get(id) : 0.f;
        };
        transform["getPositionY"] = [](mir::Id id) -> float {
            return mir::transform::PositionY::IsValidEntity(id) ? mir::transform::PositionY::Get(id) : 0.f;
        };
        transform["getRotation"] = [](mir::Id id) -> float {
            return mir::transform::Rotation::IsValidEntity(id) ? mir::transform::Rotation::Get(id) : 0.f;
        };
        transform["getScale"] = [](mir::Id id) -> float {
            return mir::transform::Scale::IsValidEntity(id) ? mir::transform::Scale::Get(id) : 1.f;
        };
        transform["setRotation"] = [](mir::Id id, float r) {
            mir::transform::Rotation::Set(id, r);
        };
        transform["setScale"] = [](mir::Id id, float s) {
            mir::transform::Scale::Set(id, s);
        };
        lua["Transform"] = transform;

        // Bind Key Enum
        lua.new_enum<mir::Key>("Key", {
            {"Escape", mir::Key::Escape},
            {"Space", mir::Key::Space},
            {"Enter", mir::Key::Enter},
            {"Up", mir::Key::Up},
            {"Left", mir::Key::Left},
            {"Down", mir::Key::Down},
            {"Right", mir::Key::Right},
            {"Tab", mir::Key::Tab},
            {"Lshift", mir::Key::Lshift},
            {"Lctrl", mir::Key::Lctrl},
            {"Lalt", mir::Key::Lalt},
            {"Rshift", mir::Key::Rshift},
            {"Rctrl", mir::Key::Rctrl},
            {"Ralt", mir::Key::Ralt},
            {"Q", mir::Key::Q},
            {"W", mir::Key::W},
            {"E", mir::Key::E},
            {"R", mir::Key::R},
            {"T", mir::Key::T},
            {"Y", mir::Key::Y},
            {"U", mir::Key::U},
            {"I", mir::Key::I},
            {"O", mir::Key::O},
            {"P", mir::Key::P},
            {"A", mir::Key::A},
            {"S", mir::Key::S},
            {"D", mir::Key::D},
            {"F", mir::Key::F},
            {"G", mir::Key::G},
            {"H", mir::Key::H},
            {"J", mir::Key::J},
            {"K", mir::Key::K},
            {"L", mir::Key::L},
            {"Z", mir::Key::Z},
            {"X", mir::Key::X},
            {"C", mir::Key::C},
            {"V", mir::Key::V},
            {"B", mir::Key::B},
            {"N", mir::Key::N},
            {"M", mir::Key::M}
        });

        // Bind Input functions
        auto input = lua.create_table();
        input["isPressed"] = [](mir::Key key) -> bool {
            return mir::input::IsPressed(key);
        };
        input["isJustPressed"] = [](mir::Key key) -> bool {
            return mir::input::IsJustPressed(key);
        };
        input["isJustReleased"] = [](mir::Key key) -> bool {
            return mir::input::IsJustReleased(key);
        };
        input["getMouseX"] = &mir::input::GetMouseX;
        input["getMouseY"] = &mir::input::GetMouseY;
        lua["Input"] = input;

        // Load entrypoint script
        try {
            auto result = lua.safe_script_file("script/main.lua");
            if (!result.valid()) {
                sol::error err = result;
                mir::debug::Log("Failed to run script/main.lua: %s", err.what());
                return;
            }
        } catch (const std::exception& e) {
            mir::debug::Log("Exception while running script/main.lua: %s", e.what());
            return;
        }

        // Fetch lifecycle functions
        sol::protected_function luaInit = lua["Init"];
        luaUpdate = lua["Update"];

        if (luaInit.valid()) {
            auto result = luaInit();
            if (!result.valid()) {
                sol::error err = result;
                mir::debug::Log("Error in Init(): %s", err.what());
            }
        } else {
            mir::debug::Log("Warning: Init() function not found in script/main.lua");
        }
    }

    void ScriptSystem::Update(float deltaTime) {
        if (luaUpdate.valid()) {
            auto result = luaUpdate(deltaTime);
            if (!result.valid()) {
                sol::error err = result;
                mir::debug::Log("Error in Update(): %s", err.what());
            }
        }
    }

    void ScriptSystem::Shutdown() {
        sol::protected_function luaShutdown = lua["Shutdown"];
        if (luaShutdown.valid()) {
            auto result = luaShutdown();
            if (!result.valid()) {
                sol::error err = result;
                mir::debug::Log("Error in Shutdown(): %s", err.what());
            }
        }
        // Force garbage collection and reset
        lua.collect_garbage();
    }
}
