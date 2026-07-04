#pragma once

#include <container/SparseSet.hpp>
#include "Entity.hpp"
#include "Manager.hpp"

using namespace zet;

namespace mir {
	template<typename Derived, typename Type>
	class Component {
	public:
		static constexpr bool IsValidEntity(const Id id) noexcept {
			return storage.Contains(id);
		}

		static constexpr const Type& Get(const Id id) noexcept {
			return storage.Get(id);
		}

		static constexpr void Remove(const Id id) noexcept {
			storage.Remove(id);
		}

		static constexpr void Set(const Id id, const Type& data) noexcept {
			Payload payload{id, data};
			mir::core::Manager::Instance().AddComponent<Payload>(&apply, payload, &Remove);
		}

	protected:
		struct Payload {
			mir::Id Id;
			Type Data;
		};

		static inline SparseSet<Type, MAX_ID> storage;

		static inline void apply(const void* rawPayload) {
			const Payload& payload = *static_cast<const Payload*>(rawPayload);
			storage.Assign(payload.Id, payload.Data);
		}
	};
}
