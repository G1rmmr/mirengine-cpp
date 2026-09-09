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

		[[nodiscard]] bool AddSystem(SystemFunc system) noexcept {
			if (system == nullptr || systemFuncs.Size() >= MAX_SYSTEM) {
				return false;
			}
			systemFuncs.Push(system);
			return true;
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

		bool HasCleanup(CleanupFunc cleanup) const noexcept {
			for (CleanupFunc exist : cleanupFuncs) {
				if (exist == cleanup) return true;
			}
			return false;
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
