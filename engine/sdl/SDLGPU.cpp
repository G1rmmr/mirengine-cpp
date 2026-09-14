#include "../device/GPUDevice.hpp"
#include "SDLInternal.hpp"

namespace mir {
	GPUDevice GPUDevice::Create(const std::uint32_t formats, const bool debugMode) noexcept {
		return GPUDevice{SDL_CreateGPUDevice(formats, debugMode, nullptr)};
	}

	bool GPUDevice::ClaimWindow() const noexcept {
		return device != nullptr && sdl::sdlWindow != nullptr && SDL_ClaimWindowForGPUDevice(device, sdl::sdlWindow);
	}

	void GPUDevice::ReleaseWindow() const noexcept {
		if (device != nullptr && sdl::sdlWindow != nullptr) SDL_ReleaseWindowFromGPUDevice(device, sdl::sdlWindow);
	}

	int GPUDevice::GetSwapchainFormat() const noexcept {
		if (device == nullptr || sdl::sdlWindow == nullptr) return 0;
		return static_cast<int>(SDL_GetGPUSwapchainTextureFormat(device, sdl::sdlWindow));
	}

	void GPUDevice::Destroy() noexcept {
		if (device == nullptr) return;
		SDL_DestroyGPUDevice(device);
		device = nullptr;
	}
}
