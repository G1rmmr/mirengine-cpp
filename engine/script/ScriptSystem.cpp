#include "ScriptSystem.hpp"
#include "../core/Manager.hpp"
#include "../component/Transform.hpp"
#include "../component/Sprite.hpp"
#include "../component/Rigidbody.hpp"
#include "../component/Collider.hpp"
#include "../component/Tag.hpp"
#include "../device/Input.hpp"
#include "../device/Window.hpp"
#include "../device/Key.hpp"
#include "../system/Movement.hpp"
#include "../system/Collision.hpp"
#include "../asset/Sound.hpp"
#include "../asset/Texture.hpp"
#include "../asset/Font.hpp"
#include "../util/Debugger.hpp"
#include <container/List.hpp>

#include <iostream>
#include <string>

namespace mir::script {

    void ScriptSystem::Initialize() {
        // Open default libraries
        lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::table, sol::lib::string, sol::lib::math);

        // Bind Debug Log override for print
        lua.set_function("print", [](sol::variadic_args va) {
            std::string logMsg = "";
            for (auto target : va) {
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

        // Bind Sound functions
        auto sound = lua.create_table();
        sound["Load"] = [](const std::string& name) -> bool {
            return mir::sound::Load(name.c_str());
        };
        sound["Play"] = [](const std::string& name, float volume, float pitch) {
            mir::sound::Play(name.c_str(), volume, pitch);
        };
        sound["PlayAt"] = [](const std::string& name, float x, float y, float volume, float pitch) {
            mir::sound::PlayAt(name.c_str(), x, y, volume, pitch);
        };
        sound["PlayBgm"] = [](const std::string& name, float volume, bool loop) {
            mir::sound::PlayBgm(name.c_str(), volume, loop);
        };
        sound["StopBgm"] = &mir::sound::StopBgm;
        sound["SetBgmVolume"] = &mir::sound::SetBgmVolume;
        sound["SetMasterVolume"] = &mir::sound::SetMasterVolume;
        sound["StopAll"] = &mir::sound::StopAll;
        lua["Sound"] = sound;

        // Bind Texture functions
        auto texture = lua.create_table();
        texture["Load"] = [](const std::string& name) -> bool {
            return mir::texture::Load(name.c_str());
        };
        lua["Texture"] = texture;

        // Bind Font functions
        auto font = lua.create_table();
        font["Load"] = [](const std::string& name) -> bool {
            return mir::font::Load(name.c_str());
        };
        lua["Font"] = font;

        // Bind Transform Component functions
        auto transform = lua.create_table();
        transform["SetPosition"] = &mir::transform::SetPosition;
        transform["GetPositionX"] = [](mir::Id id) -> float {
            return mir::transform::PositionX::IsValidEntity(id) ? mir::transform::PositionX::Get(id) : 0.f;
        };
        transform["GetPositionY"] = [](mir::Id id) -> float {
            return mir::transform::PositionY::IsValidEntity(id) ? mir::transform::PositionY::Get(id) : 0.f;
        };
        transform["GetRotation"] = [](mir::Id id) -> float {
            return mir::transform::Rotation::IsValidEntity(id) ? mir::transform::Rotation::Get(id) : 0.f;
        };
        transform["GetScale"] = [](mir::Id id) -> float {
            return mir::transform::Scale::IsValidEntity(id) ? mir::transform::Scale::Get(id) : 1.f;
        };
        transform["SetRotation"] = [](mir::Id id, float r) {
            mir::transform::Rotation::Set(id, r);
        };
        transform["SetScale"] = [](mir::Id id, float s) {
            mir::transform::Scale::Set(id, s);
        };
        transform["IsValid"] = [](mir::Id id) -> bool {
            return mir::transform::PositionX::IsValidEntity(id);
        };
        transform["Remove"] = [](mir::Id id) {
            mir::transform::PositionX::Remove(id);
            mir::transform::PositionY::Remove(id);
            mir::transform::Rotation::Remove(id);
            mir::transform::Scale::Remove(id);
        };
        lua["Transform"] = transform;

        // Bind Sprite Component functions
        auto sprite = lua.create_table();
        sprite["SetTexture"] = [](mir::Id id, const std::string& path) {
            mir::sprite::Texture::Set(id, path.c_str());
        };
        sprite["GetTexture"] = [](mir::Id id) -> std::string {
            return mir::sprite::Texture::IsValidEntity(id) ? mir::sprite::Texture::Get(id).c_str() : "";
        };
        sprite["SetSourceSize"] = &mir::sprite::SetSourceSize;
        sprite["SetDestinationSize"] = &mir::sprite::SetDestinationSize;
        sprite["SetAnchor"] = &mir::sprite::SetAnchor;
        sprite["SetTint"] = &mir::sprite::SetTint;
        sprite["SetZindex"] = [](mir::Id id, std::uint16_t z) {
            mir::sprite::Zindex::Set(id, z);
        };
        sprite["GetZindex"] = [](mir::Id id) -> std::uint16_t {
            return mir::sprite::Zindex::IsValidEntity(id) ? mir::sprite::Zindex::Get(id) : 0;
        };
        sprite["SetAlpha"] = [](mir::Id id, std::uint8_t a) {
            mir::sprite::Alpha::Set(id, a);
        };
        sprite["GetAlpha"] = [](mir::Id id) -> std::uint8_t {
            return mir::sprite::Alpha::IsValidEntity(id) ? mir::sprite::Alpha::Get(id) : 255;
        };
        sprite["IsValid"] = [](mir::Id id) -> bool {
            return mir::sprite::Texture::IsValidEntity(id);
        };
        sprite["Remove"] = [](mir::Id id) {
            mir::sprite::Texture::Remove(id);
            mir::sprite::SourceWidth::Remove(id);
            mir::sprite::SourceHeight::Remove(id);
            mir::sprite::DestinationWidth::Remove(id);
            mir::sprite::DestinationHeight::Remove(id);
            mir::sprite::AnchorX::Remove(id);
            mir::sprite::AnchorY::Remove(id);
            mir::sprite::Zindex::Remove(id);
            mir::sprite::Alpha::Remove(id);
            mir::sprite::TintRed::Remove(id);
            mir::sprite::TintGreen::Remove(id);
            mir::sprite::TintBlue::Remove(id);
        };
        lua["Sprite"] = sprite;

        // Bind Rigidbody Component functions
        auto rb = lua.create_table();
        rb["SetVelocity"] = &mir::rigidbody::SetVelocity;
        rb["GetVelocityX"] = [](mir::Id id) -> float {
            return mir::rigidbody::VelocityX::IsValidEntity(id) ? mir::rigidbody::VelocityX::Get(id) : 0.f;
        };
        rb["GetVelocityY"] = [](mir::Id id) -> float {
            return mir::rigidbody::VelocityY::IsValidEntity(id) ? mir::rigidbody::VelocityY::Get(id) : 0.f;
        };
        rb["SetGravity"] = [](mir::Id id, float g) {
            mir::rigidbody::Gravity::Set(id, g);
        };
        rb["GetGravity"] = [](mir::Id id) -> float {
            return mir::rigidbody::Gravity::IsValidEntity(id) ? mir::rigidbody::Gravity::Get(id) : 0.f;
        };
        rb["SetOnGround"] = [](mir::Id id, bool onGround) {
            mir::rigidbody::OnGround::Set(id, onGround);
        };
        rb["IsOnGround"] = [](mir::Id id) -> bool {
            return mir::rigidbody::OnGround::IsValidEntity(id) ? mir::rigidbody::OnGround::Get(id) : false;
        };
        rb["IsValid"] = [](mir::Id id) -> bool {
            return mir::rigidbody::VelocityX::IsValidEntity(id);
        };
        rb["Remove"] = [](mir::Id id) {
            mir::rigidbody::VelocityX::Remove(id);
            mir::rigidbody::VelocityY::Remove(id);
            mir::rigidbody::Gravity::Remove(id);
            mir::rigidbody::OnGround::Remove(id);
        };
        lua["Rigidbody"] = rb;

        // Bind Collider Component functions
        auto col = lua.create_table();
        col["SetBound"] = &mir::collider::SetBound;
        col["GetBoundX"] = [](mir::Id id) -> float {
            return mir::collider::BoundX::IsValidEntity(id) ? mir::collider::BoundX::Get(id) : 0.f;
        };
        col["GetBoundY"] = [](mir::Id id) -> float {
            return mir::collider::BoundY::IsValidEntity(id) ? mir::collider::BoundY::Get(id) : 0.f;
        };
        col["SetOffset"] = &mir::collider::SetOffset;
        col["GetOffsetX"] = [](mir::Id id) -> float {
            return mir::collider::OffsetX::IsValidEntity(id) ? mir::collider::OffsetX::Get(id) : 0.f;
        };
        col["GetOffsetY"] = [](mir::Id id) -> float {
            return mir::collider::OffsetY::IsValidEntity(id) ? mir::collider::OffsetY::Get(id) : 0.f;
        };
        col["SetShouldTrigger"] = [](mir::Id id, bool trigger) {
            mir::collider::ShouldTrigger::Set(id, trigger);
        };
        col["GetShouldTrigger"] = [](mir::Id id) -> bool {
            return mir::collider::ShouldTrigger::IsValidEntity(id) ? mir::collider::ShouldTrigger::Get(id) : false;
        };
        col["IsValid"] = [](mir::Id id) -> bool {
            return mir::collider::BoundX::IsValidEntity(id);
        };
        col["Remove"] = [](mir::Id id) {
            mir::collider::BoundX::Remove(id);
            mir::collider::BoundY::Remove(id);
            mir::collider::OffsetX::Remove(id);
            mir::collider::OffsetY::Remove(id);
            mir::collider::ShouldTrigger::Remove(id);
        };
        lua["Collider"] = col;

        // Bind Tag Component functions (mir::event)
        auto tag = lua.create_table();
        tag["Set"] = [](mir::Id id, const std::string& name) {
            mir::event::SetTag(id, name.c_str());
        };
        tag["Get"] = [](mir::Id id) -> std::string {
            return mir::event::Tag::IsValidEntity(id) ? mir::event::Tag::Get(id).c_str() : "";
        };
        tag["IsValid"] = [](mir::Id id) -> bool {
            return mir::event::Tag::IsValidEntity(id);
        };
        tag["Remove"] = [](mir::Id id) {
            mir::event::Tag::Remove(id);
        };
        lua["Tag"] = tag;

        // Bind Movement System
        auto movement = lua.create_table();
        movement["Update"] = &mir::movement::Update;
        lua["Movement"] = movement;

        // Bind Collision System
        auto collision = lua.create_table();
        collision["Update"] = &mir::collision::Update;
        lua["Collision"] = collision;

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
        input["IsPressed"] = [](mir::Key key) -> bool {
            return mir::input::IsPressed(key);
        };
        input["IsJustPressed"] = [](mir::Key key) -> bool {
            return mir::input::IsJustPressed(key);
        };
        input["IsJustReleased"] = [](mir::Key key) -> bool {
            return mir::input::IsJustReleased(key);
        };
        input["GetMouseX"] = &mir::input::GetMouseX;
        input["GetMouseY"] = &mir::input::GetMouseY;
        lua["Input"] = input;

        // Bind Window functions
        auto window = lua.create_table();
        window["Close"] = &mir::window::Close;
        lua["Window"] = window;

        // Bind zet::List types
        lua.new_usertype<zet::List<int, 100>>("ListInt",
            sol::constructors<zet::List<int, 100>()>(),
            "push", [](zet::List<int, 100>& list, int val) { list.Push(val); },
            "pop", &zet::List<int, 100>::Pop,
            "clear", &zet::List<int, 100>::Clear,
            "size", &zet::List<int, 100>::Size,
            "capacity", &zet::List<int, 100>::Capacity,
            "get", [](zet::List<int, 100>& list, std::size_t luaIdx) -> sol::optional<int> {
                if (luaIdx < 1 || luaIdx > list.Size()) return sol::nullopt;
                return list[luaIdx - 1];
            },
            "set", [](zet::List<int, 100>& list, std::size_t luaIdx, int val) -> bool {
                if (luaIdx < 1 || luaIdx > list.Size()) return false;
                list[luaIdx - 1] = val;
                return true;
            }
        );

        lua.new_usertype<zet::List<float, 100>>("ListFloat",
            sol::constructors<zet::List<float, 100>()>(),
            "push", [](zet::List<float, 100>& list, float val) { list.Push(val); },
            "pop", &zet::List<float, 100>::Pop,
            "clear", &zet::List<float, 100>::Clear,
            "size", &zet::List<float, 100>::Size,
            "capacity", &zet::List<float, 100>::Capacity,
            "get", [](zet::List<float, 100>& list, std::size_t luaIdx) -> sol::optional<float> {
                if (luaIdx < 1 || luaIdx > list.Size()) return sol::nullopt;
                return list[luaIdx - 1];
            },
            "set", [](zet::List<float, 100>& list, std::size_t luaIdx, float val) -> bool {
                if (luaIdx < 1 || luaIdx > list.Size()) return false;
                list[luaIdx - 1] = val;
                return true;
            }
        );

        lua.new_usertype<zet::List<std::string, 100>>("ListString",
            sol::constructors<zet::List<std::string, 100>()>(),
            "push", [](zet::List<std::string, 100>& list, std::string val) { list.Push(val); },
            "pop", &zet::List<std::string, 100>::Pop,
            "clear", &zet::List<std::string, 100>::Clear,
            "size", &zet::List<std::string, 100>::Size,
            "capacity", &zet::List<std::string, 100>::Capacity,
            "get", [](zet::List<std::string, 100>& list, std::size_t luaIdx) -> sol::optional<std::string> {
                if (luaIdx < 1 || luaIdx > list.Size()) return sol::nullopt;
                return list[luaIdx - 1];
            },
            "set", [](zet::List<std::string, 100>& list, std::size_t luaIdx, std::string val) -> bool {
                if (luaIdx < 1 || luaIdx > list.Size()) return false;
                list[luaIdx - 1] = val;
                return true;
            }
        );

        // Load entrypoint script
        try {
            auto result = lua.safe_script_file("main.lua");
            if (!result.valid()) {
                sol::error err = result;
                mir::debug::Log("Failed to run main.lua: %s", err.what());
                return;
            }
        } catch (const std::exception& e) {
            mir::debug::Log("Exception while running main.lua: %s", e.what());
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
