#pragma once

#include <SDL3/SDL.h>

#include <cstdint>

namespace mir {
	// Lua receives this owner-aware object rather than a raw SDL pointer. C++
	// users can still obtain Raw() when integrating lower-level SDL GPU code.
	class GPUDevice {
	public:
		GPUDevice() = default;
		explicit GPUDevice(SDL_GPUDevice* device) noexcept : device(device) {}
		~GPUDevice() { Destroy(); }

		GPUDevice(const GPUDevice&) = delete;
		GPUDevice& operator=(const GPUDevice&) = delete;

		GPUDevice(GPUDevice&& other) noexcept : device(other.device) { other.device = nullptr; }
		GPUDevice& operator=(GPUDevice&& other) noexcept {
			if (this == &other) return *this;
			Destroy();
			device = other.device;
			other.device = nullptr;
			return *this;
		}

		[[nodiscard]] static GPUDevice Create(std::uint32_t formats, bool debugMode) noexcept;
		[[nodiscard]] bool IsValid() const noexcept { return device != nullptr; }
		[[nodiscard]] bool ClaimWindow() const noexcept;
		void ReleaseWindow() const noexcept;
		[[nodiscard]] int GetSwapchainFormat() const noexcept;
		void Destroy() noexcept;

		[[nodiscard]] SDL_GPUDevice* Raw() const noexcept { return device; }

	private:
		SDL_GPUDevice* device = nullptr;
	};
}
