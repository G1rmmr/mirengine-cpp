#pragma once

#include <sol/sol.hpp>
#include "../core/Entity.hpp"
#include "../core/Manager.hpp"
#include <container/String.hpp>

namespace mir::script {
	struct ScriptScheduler;

	// This is intentionally separate from core::SystemId. Lua cannot forge or
	// unwrap either handle, while C++ systems remain registered in Manager.
	struct ScriptSystemId {
		constexpr ScriptSystemId() noexcept = default;
		constexpr bool operator==(const ScriptSystemId&) const noexcept = default;

	private:
		PoolHandle handle{};
		core::SystemPhase phase = core::SystemPhase::Simulation;

		constexpr ScriptSystemId(const PoolHandle value, const core::SystemPhase valuePhase) noexcept
			: handle(value), phase(valuePhase) {}
		friend class ScriptSystem;
		friend struct ScriptScheduler;
	};

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

		bool Initialize();
        void Update(float deltaTime);
        void Shutdown();

		[[nodiscard]] ScriptSystemId RegisterSystem(sol::protected_function callback, core::SystemPhase phase = core::SystemPhase::Simulation);
		[[nodiscard]] bool AddSystemDependency(ScriptSystemId before, ScriptSystemId after);
		[[nodiscard]] bool IsValidSystem(ScriptSystemId id) const noexcept;
		[[nodiscard]] bool OnEvent(const String<>& name, sol::protected_function callback);
		void ClearEventListeners() noexcept;

        sol::state& GetLuaState() noexcept { return lua; }

    private:
		ScriptSystem() = default;
		~ScriptSystem() = default;

		static void DispatchSimulation(float deltaTime);
		static void DispatchPostCommit(float deltaTime);
		static void DispatchEvent(Id id, const String<>& name) noexcept;

        sol::state lua;
        sol::protected_function luaUpdate;
    };
}
