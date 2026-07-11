#pragma once
#include <SDL3/SDL.h>
#include <fstream>
#include <vector>

namespace mir {
    class Shader {
    public:
        Shader() : m_shader(nullptr) {}
        ~Shader() { Destroy(); }

        bool LoadFromBytecode(SDL_GPUDevice* device, const std::uint8_t* code, std::size_t size, 
                              const char* entrypoint, SDL_GPUShaderStage stage,
                              std::uint32_t numSamplers = 0, std::uint32_t numUniformBuffers = 0) noexcept {
            Destroy();

            SDL_GPUShaderCreateInfo createInfo{};
            createInfo.code_size = size;
            createInfo.code = code;
            createInfo.entrypoint = entrypoint;
            createInfo.stage = stage;
            createInfo.num_samplers = numSamplers;
            createInfo.num_uniform_buffers = numUniformBuffers;

            m_device = device;
            m_shader = SDL_CreateGPUShader(device, &createInfo);
            return m_shader != nullptr;
        }

        bool LoadFromFile(SDL_GPUDevice* device, const char* filepath,
                          const char* entrypoint, SDL_GPUShaderStage stage,
                          std::uint32_t numSamplers = 0, std::uint32_t numUniformBuffers = 0) noexcept {
            std::ifstream file(filepath, std::ios::binary | std::ios::ate);
            if (!file.is_open()) return false;

            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            std::vector<std::uint8_t> buffer(size);
            if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) return false;

            return LoadFromBytecode(device, buffer.data(), buffer.size(), entrypoint, stage, numSamplers, numUniformBuffers);
        }

        void Destroy() noexcept {
            if (m_shader && m_device) {
                SDL_ReleaseGPUShader(m_device, m_shader);
                m_shader = nullptr;
            }
        }

        SDL_GPUShader* GetRawShader() const noexcept { return m_shader; }

    private:
        SDL_GPUDevice* m_device = nullptr;
        SDL_GPUShader* m_shader = nullptr;
    };
}

