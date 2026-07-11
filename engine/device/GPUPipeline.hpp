#pragma once
#include "Shader.hpp"
#include <SDL3/SDL.h>

namespace mir {
    class GPUPipeline {
    public:
        GPUPipeline() : m_pipeline(nullptr), m_device(nullptr) {}
        ~GPUPipeline() { Destroy(); }

        bool Create(SDL_GPUDevice* device, 
                    const Shader& vertexShader, 
                    const Shader& fragmentShader,
                    SDL_GPUTextureFormat renderTargetFormat) noexcept {
            Destroy();
            m_device = device;

            // Simple vertex input / pipeline layout setup
            SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{};
            
            // Shaders
            pipelineInfo.vertex_shader = vertexShader.GetRawShader();
            pipelineInfo.fragment_shader = fragmentShader.GetRawShader();

            // Primitive topology
            pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

            // Render Target / Color Attachment
            SDL_GPUColorTargetDescription colorTargetDesc{};
            colorTargetDesc.format = renderTargetFormat;
            colorTargetDesc.blend_state.enable_blend = true;
            colorTargetDesc.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
            colorTargetDesc.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
            colorTargetDesc.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
            colorTargetDesc.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
            colorTargetDesc.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
            colorTargetDesc.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;

            pipelineInfo.target_info.num_color_targets = 1;
            pipelineInfo.target_info.color_target_descriptions = &colorTargetDesc;

            m_pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineInfo);
            return m_pipeline != nullptr;
        }

        void Destroy() noexcept {
            if (m_pipeline && m_device) {
                SDL_ReleaseGPUGraphicsPipeline(m_device, m_pipeline);
                m_pipeline = nullptr;
            }
        }

        SDL_GPUGraphicsPipeline* GetRawPipeline() const noexcept { return m_pipeline; }

    private:
        SDL_GPUGraphicsPipeline* m_pipeline = nullptr;
        SDL_GPUDevice* m_device = nullptr;
    };
}
