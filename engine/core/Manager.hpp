#pragma once

#include "Entity.hpp"

#include <container/CommandBuffer.hpp>
#include <container/Pool.hpp>
#include <container/List.hpp>

#if __has_include("Config.hpp")
	#include "Config.hpp"
#endif

using namespace zet;

namespace mir::core {
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
			return availablePool.Create(true);
		}

			bool DeleteEntity(const Id id) noexcept {
				if (!IsValidEntity(id)) return false;
			struct Payload {
				Id EntityId;
			};

			Payload payload{ id };

			auto apply = [](const void* rawPayload) {
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
			if (availablePool.IsOccupied(index)) {
				return Id(index, availablePool.GetGeneration(index));
			}
			return INVALID_ID;
		}

			template<typename Payload>
			bool AddComponent(void (*apply)(const void*), const Payload& payload, CleanupFunc cleanup) noexcept {
				bool cleanupKnown = false;
				for (CleanupFunc exist : cleanupFuncs) {
					if (exist == cleanup) {
						cleanupKnown = true;
						break;
					}
				}

				if (!cleanupKnown && cleanupFuncs.Size() >= MAX_COMPONENT) {
					++droppedCommands;
					return false;
				}

				if (!commandBuffer.Push<Payload>(apply, payload)) {
					++droppedCommands;
					return false;
				}

				if (!cleanupKnown) {
					cleanupFuncs.Push(cleanup);
				}
				return true;
		}

		void AddSystem(SystemFunc system) noexcept {
			if (systemFuncs.Size() < MAX_SYSTEM) {
				systemFuncs.Push(system);
			}
		}

		void UpdateSystem(const float deltaTime) noexcept {
			for (SystemFunc update : systemFuncs) {
				update(deltaTime);
			}
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

			CommandBuffer<mir::config::COMMAND_BUFFER_BYTES> commandBuffer;
			std::size_t droppedCommands = 0;

		Pool<bool, MAX_ID> availablePool;

		List<CleanupFunc, MAX_COMPONENT> cleanupFuncs{};
		List<SystemFunc, MAX_SYSTEM> systemFuncs{};

		Manager() noexcept {}
		~Manager() = default;

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
