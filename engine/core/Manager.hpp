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

		void DeleteEntity(const Id id) noexcept {
			struct Payload {
				Id EntityId;
			};

			Payload payload{ id };

			auto apply = [](const void* rawPayload) {
				const auto& p = *static_cast<const Payload*>(rawPayload);
				Manager::Instance().destroyEntity(p.EntityId);
			};

			commandBuffer.Push<Payload>(apply, payload);
		}

		bool IsValidEntity(const Id id) const noexcept {
			if (id.Index >= MAX_ID) {
				return false;
			}
			return availablePool.IsValid(id);
		}

		template<typename Payload>
		void AddComponent(void (*apply)(const void*), const Payload& payload, CleanupFunc cleanup) noexcept {
			commandBuffer.Push<Payload>(apply, payload);

			for (CleanupFunc exist : cleanupFuncs) {
				if(exist == cleanup) {
					return;
				}
			}

			if (cleanupFuncs.Size() < MAX_COMPONENT) {
				cleanupFuncs.Push(cleanup);
			}
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

		CommandBuffer<> commandBuffer;

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
