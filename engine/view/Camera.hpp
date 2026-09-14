#pragma once

#include "../component/Transform.hpp"

#include <algorithm>
#include <cmath>

namespace mir::camera {
	inline float PositionX = 0.f;
	inline float PositionY = 0.f;
	inline float CurrentZoom = 1.f;
	inline float ShakePower = 0.f;
	inline float ShakeTime = 0.f;
	inline Id Target = INVALID_ID;
	inline core::SystemId CameraSystem{};

	inline void Update(const float deltaTime) noexcept {
		if (core::Manager::Instance().IsValidEntity(Target)) {
			const float* worldX = transform::WorldPositionX::TryGet(Target);
			const float* worldY = transform::WorldPositionY::TryGet(Target);
			const float* localX = transform::PositionX::TryGet(Target);
			const float* localY = transform::PositionY::TryGet(Target);
			if (worldX != nullptr) PositionX = *worldX;
			else if (localX != nullptr) PositionX = *localX;
			if (worldY != nullptr) PositionY = *worldY;
			else if (localY != nullptr) PositionY = *localY;
		} else if (Target != INVALID_ID) {
			Target = INVALID_ID;
		}
		ShakeTime = std::max(0.f, ShakeTime - std::max(0.f, deltaTime));
		if (ShakeTime == 0.f) ShakePower = 0.f;
	}

	inline void UpdateSystem(const float deltaTime) noexcept { Update(deltaTime); }

	[[nodiscard]] inline bool EnsureSystem() noexcept {
		auto& manager = core::Manager::Instance();
		if (manager.IsValidSystem(CameraSystem)) return true;
		CameraSystem = manager.RegisterSystem(&UpdateSystem, core::SystemPhase::PostCommit);
		return manager.IsValidSystem(CameraSystem);
	}

	inline void SetPosition(const float x, const float y) noexcept {
		PositionX = x;
		PositionY = y;
		Target = INVALID_ID;
	}

	[[nodiscard]] inline bool Follow(const Id target) noexcept {
		if (!core::Manager::Instance().IsValidEntity(target) || !EnsureSystem()) return false;
		Target = target;
		return true;
	}

	inline void ClearFollow() noexcept { Target = INVALID_ID; }
	inline void SetZoom(const float zoom) noexcept { CurrentZoom = std::max(zoom, 0.001f); }
	inline void Shake(const float intensity, const float duration) noexcept {
		ShakePower = std::max(0.f, intensity);
		ShakeTime = std::max(0.f, duration);
	}

	[[nodiscard]] inline float GetX() noexcept { return PositionX; }
	[[nodiscard]] inline float GetY() noexcept { return PositionY; }
	[[nodiscard]] inline float GetZoom() noexcept { return CurrentZoom; }
	[[nodiscard]] inline float GetShakePower() noexcept { return ShakePower; }
	[[nodiscard]] inline float GetShakeTime() noexcept { return ShakeTime; }
	[[nodiscard]] inline float GetShakeOffsetX() noexcept { return ShakeTime > 0.f ? std::sin(ShakeTime * 73.f) * ShakePower : 0.f; }
	[[nodiscard]] inline float GetShakeOffsetY() noexcept { return ShakeTime > 0.f ? std::cos(ShakeTime * 59.f) * ShakePower : 0.f; }
	[[nodiscard]] inline Id GetTarget() noexcept { return Target; }
}
