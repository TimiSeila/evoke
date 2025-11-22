#pragma once
#define GLFW_EXPOSE_NATIVE_COCOA
#define VK_USE_PLATFORM_METAL_EXT
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vector>

#include "VulkanSwapchain.h"

namespace evoke::vulkan {
    class Pipeline{
    public:
        void create_pipeline(VkDevice device, VulkanSwapchain& swapchain);
        void clean_up(VkDevice device);
        
        VkPipeline get_graphics_pipeline() { return m_graphics_pipeline; }
    private:
        VkPipelineLayout m_pipeline_layout;
        VkPipeline m_graphics_pipeline;
        
        VkShaderModule create_shader_module(const std::vector<char>& bytecode, VkDevice device);
    };
}
