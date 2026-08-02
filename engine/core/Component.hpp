#pragma once

#include <container/SparseSet.hpp>
#include "Entity.hpp"
#include "Manager.hpp"
#include <cassert>

using namespace zet;

namespace mir {
	template<typename Derived, typename Type>
	class Component {
	public:
			static bool IsValidEntity(const Id id) noexcept {
				if (!mir::core::Manager::Instance().IsValidEntity(id) || !storage.Contains(id.Index)) {
					return false;
				}
				return storage.Get(id.Index).Generation == id.Generation;
			}

			static const Type& Get(const Id id) noexcept {
				assert(IsValidEntity(id) && "[mir::Component] INVALID OR STALE ENTITY");
				return storage.Get(id.Index).Data;
			}

			static void Remove(const Id id) noexcept {
				if (storage.Contains(id.Index) && storage.Get(id.Index).Generation == id.Generation) {
					storage.Remove(id.Index);
				}
			}

			static bool Set(const Id id, const Type& data) noexcept {
				if (!mir::core::Manager::Instance().IsValidEntity(id)) return false;
				Payload payload{id, data};
				return mir::core::Manager::Instance().AddComponent<Payload>(&apply, payload, &Remove);
			}

	protected:
			struct Payload {
				mir::Id Id;
				Type Data;
			};

			struct Record {
				std::size_t Generation;
				Type Data;
			};

			static inline SparseSet<Record, MAX_ID> storage;

			static inline void apply(const void* rawPayload) {
				const Payload& payload = *static_cast<const Payload*>(rawPayload);
				if (mir::core::Manager::Instance().IsValidEntity(payload.Id)) {
					storage.Assign(payload.Id.Index, Record{payload.Id.Generation, payload.Data});
				}
			}
	};
}
