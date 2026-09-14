#pragma once

#include "Entity.hpp"

#include <container/CommandBuffer.hpp>
#include <container/FixedGraph.hpp>
#include <container/Pool.hpp>
#include <container/List.hpp>

#include <array>
#include <span>

#if __has_include("Config.hpp")
	#include "Config.hpp"
#endif

using namespace zet;

namespace mir::core {
	enum class SystemPhase {
		Simulation,
		PostCommit
	};

	// A distinct wrapper prevents ZET graph handles from being accidentally
	// treated as entity Ids. Only Manager can unwrap the handle.
	struct SystemId {
		constexpr SystemId() noexcept = default;
		constexpr bool operator==(const SystemId&) const noexcept = default;

	private:
		PoolHandle handle{};

		explicit constexpr SystemId(const PoolHandle value) noexcept : handle(value) {}
		friend class Manager;
	};

	class Manager {
	public:
		using CleanupFunc = void (*)(Id);
		using SystemFunc = void (*)(float);

		Manager(const Manager&) = delete;
		Manager& operator=(const Manager&) = delete;
		Manager(Manager&&) = delete;
		Manager& operator=(Manager&&) = delete;

		static Manager& Instance() noexcept {
			static Manager instance;
			return instance;
		}

		Id AddEntity() noexcept {
			return Id(availablePool.TryCreate(true));
		}

		bool DeleteEntity(const Id id) noexcept {
			if (!IsValidEntity(id)) return false;
			struct Payload {
				Id EntityId;
			};

			Payload payload{ id };

			auto apply = [](const void* rawPayload) noexcept {
				const auto& p = *static_cast<const Payload*>(rawPayload);
				Manager::Instance().destroyEntity(p.EntityId);
			};

			if (!commandBuffer.Push<Payload>(apply, payload)) {
				++droppedCommands;
				return false;
			}
			return true;
		}

		bool IsValidEntity(const Id id) const noexcept {
			if (id.Index >= MAX_ID) {
				return false;
			}
			return availablePool.IsValid(id);
		}

		Id GetActiveEntityId(const std::size_t index) const noexcept {
			if (index >= MAX_ID) {
				return INVALID_ID;
			}
			return Id(availablePool.TryHandleAt(index));
		}

		template<typename Payload>
		bool AddComponent(void (*apply)(const void*) noexcept, const Payload& payload, CleanupFunc cleanup) noexcept {
			if (!HasCleanup(cleanup) && cleanupFuncs.Size() >= MAX_COMPONENT) {
				++droppedCommands;
				return false;
			}

			if (!Enqueue<Payload>(apply, payload)) return false;

			if (!HasCleanup(cleanup)) {
				cleanupFuncs.Push(cleanup);
			}
			return true;
		}

		[[nodiscard]] bool RegisterCleanup(CleanupFunc cleanup) noexcept {
			if (cleanup == nullptr || HasCleanup(cleanup)) return cleanup != nullptr;
			if (cleanupFuncs.Size() >= MAX_COMPONENT) {
				++droppedCommands;
				return false;
			}
			cleanupFuncs.Push(cleanup);
			return true;
		}

		template<typename Payload>
		[[nodiscard]] bool Enqueue(void (*apply)(const void*) noexcept, const Payload& payload) noexcept {
			if (!commandBuffer.Push<Payload>(apply, payload)) {
				++droppedCommands;
				return false;
			}
			return true;
		}

		// Existing callers can keep registering a simulation system with this
		// simple API. New systems that need ordering should retain the returned
		// SystemId from RegisterSystem and add explicit dependencies.
		[[nodiscard]] bool AddSystem(SystemFunc system) noexcept {
			return IsValidSystem(RegisterSystem(system));
		}

		[[nodiscard]] SystemId RegisterSystem(
			SystemFunc system,
			const SystemPhase phase = SystemPhase::Simulation) noexcept {
			if (system == nullptr) return {};

			for (const SystemRegistration& registered : registeredSystems) {
				if (registered.Func == system) {
					return registered.Phase == phase ? registered.Id : SystemId{};
				}
			}

			const PoolHandle handle = systemGraph.TryAddVertex(SystemRecord{system, phase});
			if (!systemGraph.IsValidVertex(handle) || !registeredSystems.TryPush(SystemRegistration{SystemId{handle}, system, phase})) {
				return {};
			}

			if (!RebuildSystemSchedule()) {
				// A vertex without edges cannot introduce a cycle. Keep this guard so
				// a future graph implementation cannot leave a stale schedule behind.
				return {};
			}
			return SystemId{handle};
		}

		[[nodiscard]] bool AddSystemDependency(const SystemId before, const SystemId after) noexcept {
			if (!IsValidSystem(before) || !IsValidSystem(after) || before == after) return false;

			const SystemRecord* beforeRecord = systemGraph.TryGetVertex(before.handle);
			const SystemRecord* afterRecord = systemGraph.TryGetVertex(after.handle);
			// Phases are commit barriers. Ordering edges may only describe work
			// inside one phase, otherwise the graph would imply an impossible order.
			if (!beforeRecord || !afterRecord || beforeRecord->Phase != afterRecord->Phase) return false;

			const PoolHandle edge = systemGraph.TryAddEdge(before.handle, after.handle, SystemDependency{});
			if (!systemGraph.IsValidEdge(edge)) return false;
			if (RebuildSystemSchedule()) return true;

			(void)systemGraph.TryRemoveEdge(edge);
			(void)RebuildSystemSchedule();
			return false;
		}

		[[nodiscard]] bool IsValidSystem(const SystemId id) const noexcept {
			return systemGraph.IsValidVertex(id.handle);
		}

		void UpdateSystem(const float deltaTime) noexcept {
			// Commands queued by the script or the previous frame become visible to
			// simulation systems first. Post-commit systems can then consume the
			// freshly committed simulation state in the same frame.
			commandBuffer.Commit();
			RunSystems(SystemPhase::Simulation, deltaTime);
			commandBuffer.Commit();
			RunSystems(SystemPhase::PostCommit, deltaTime);
			commandBuffer.Commit();
		}

		std::size_t DroppedCommandCount() const noexcept { return droppedCommands; }

	private:
		#ifdef CONFIG_MAX_COMPONENT
		static constexpr std::size_t MAX_COMPONENT = CONFIG_MAX_COMPONENT;
		#else
		static constexpr std::size_t MAX_COMPONENT = 128;
		#endif

		#ifdef CONFIG_MAX_SYSTEM
		static constexpr std::size_t MAX_SYSTEM = CONFIG_MAX_SYSTEM;
		#else
		static constexpr std::size_t MAX_SYSTEM = 64;
		#endif

		#ifdef CONFIG_MAX_SYSTEM_DEPENDENCIES
		static constexpr std::size_t MAX_SYSTEM_DEPENDENCIES = CONFIG_MAX_SYSTEM_DEPENDENCIES;
		#else
		static constexpr std::size_t MAX_SYSTEM_DEPENDENCIES = MAX_SYSTEM * 4;
		#endif

		struct SystemDependency {};
		struct SystemRecord {
			SystemFunc Func = nullptr;
			SystemPhase Phase = SystemPhase::Simulation;
		};
		struct SystemRegistration {
			SystemId Id{};
			SystemFunc Func = nullptr;
			SystemPhase Phase = SystemPhase::Simulation;
		};
		using SystemGraph = FixedGraph<SystemRecord, SystemDependency, MAX_SYSTEM, MAX_SYSTEM_DEPENDENCIES>;

		CommandBuffer<mir::config::COMMAND_BUFFER_BYTES> commandBuffer;
		std::size_t droppedCommands = 0;

		Pool<bool, MAX_ID> availablePool;

		List<CleanupFunc, MAX_COMPONENT> cleanupFuncs{};
		SystemGraph systemGraph{};
		SystemGraph::TraversalScratch scheduleScratch{};
		List<SystemRegistration, MAX_SYSTEM> registeredSystems{};
		List<SystemFunc, MAX_SYSTEM> systemSchedule{};

		Manager() noexcept {}
		~Manager() = default;

		bool HasCleanup(CleanupFunc cleanup) const noexcept {
			for (CleanupFunc exist : cleanupFuncs) {
				if (exist == cleanup) return true;
			}
			return false;
		}

		bool RebuildSystemSchedule() noexcept {
			std::array<PoolHandle, MAX_SYSTEM> ordered{};
			const Status status = systemGraph.TopologicalSort(std::span<PoolHandle>{ordered}, scheduleScratch);
			if (status != Status::Success) return false;

			List<SystemFunc, MAX_SYSTEM> rebuilt{};
			for (std::size_t index = 0; index < systemGraph.VertexCount(); ++index) {
				const SystemRecord* record = systemGraph.TryGetVertex(ordered[index]);
				if (record == nullptr || record->Func == nullptr || !rebuilt.TryPush(record->Func)) return false;
			}
			systemSchedule = std::move(rebuilt);
			return true;
		}

		void RunSystems(const SystemPhase phase, const float deltaTime) noexcept {
			const std::size_t count = systemSchedule.Size();
			for (std::size_t index = 0; index < count; ++index) {
				const SystemFunc update = systemSchedule[index];
				for (const SystemRegistration& registered : registeredSystems) {
					if (registered.Func == update && registered.Phase == phase) {
						update(deltaTime);
						break;
					}
				}
			}
		}

		void destroyEntity(Id id) noexcept {
			if (IsValidEntity(id)) {
				for (CleanupFunc cleanup : cleanupFuncs) {
					cleanup(id);
				}
				availablePool.Destroy(id);
			}
		}
	};
}
