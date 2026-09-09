#pragma once

#include <container/String.hpp>
#include <container/Map.hpp>
#include "../component/Tag.hpp"

namespace mir::event {
	using CallbackFuncPtr = void(*)(Id);
	inline constexpr std::size_t MAX_EVENTS = 1024;
	inline constexpr std::size_t MAX_EVENT_TYPES = 256;
	inline constexpr std::size_t MAX_CALLBACKS_PER_EVENT = 16;

	struct Record {
		Id EntityId;
		String<> Name;
	};

	inline Map<String<>, List<CallbackFuncPtr, MAX_CALLBACKS_PER_EVENT>, MAX_EVENT_TYPES> callbacks;
	inline List<Record, MAX_EVENTS> pending;

	[[nodiscard]] inline bool Emit(const Id id, const String<>& name) noexcept {
		if (!mir::core::Manager::Instance().IsValidEntity(id) || pending.Size() >= MAX_EVENTS) {
			return false;
		}
		pending.Push(Record{id, name});
		return true;
	}

	inline void Update() noexcept {
		for (const Record& record : pending) {
			if (!mir::core::Manager::Instance().IsValidEntity(record.EntityId)) continue;

			auto registered = callbacks.Find(record.Name);
			if (registered != nullptr) {
				for (const auto& callback : *registered) { callback(record.EntityId); }
            }
        }
		pending.Clear();
    }

	[[nodiscard]] inline bool On(const String<>& name, CallbackFuncPtr callback) noexcept {
		if (callback == nullptr) return false;
		auto registered = callbacks.Find(name);
		if (registered == nullptr) {
			if (callbacks.Size() >= MAX_EVENT_TYPES) return false;
			registered = &callbacks.Insert(name);
		}
		if (registered->Size() >= MAX_CALLBACKS_PER_EVENT) return false;
		registered->Push(callback);
		return true;
    }
}
