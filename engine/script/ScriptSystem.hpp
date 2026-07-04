#pragma once

#include <sol/sol.hpp>
#include "../core/Entity.hpp"

namespace mir::script {
    class ScriptSystem {
    public:
        static ScriptSystem& Instance() noexcept {
            static ScriptSystem instance;
            return instance;
        }

        ScriptSystem(const ScriptSystem&) = delete;
        ScriptSystem& operator=(const ScriptSystem&) = delete;
        ScriptSystem(ScriptSystem&&) = delete;
        ScriptSystem& operator=(ScriptSystem&&) = delete;

        void Initialize();
        void Update(float deltaTime);
        void Shutdown();

        sol::state& GetLuaState() noexcept { return lua; }

    private:
        ScriptSystem() = default;
        ~ScriptSystem() = default;

        sol::state lua;
        sol::protected_function luaUpdate;
    };
}
