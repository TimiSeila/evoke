#pragma once
#include "../utils/Logger.h"
#include "VulkanSwapchain.h"
#include "VulkanPipeline.h"
#include "evPhysicalDevice.h"

namespace evoke::vulkan {
    class VulkanCore{
    public:
        void init_vulkan(GLFWwindow* window);
        void clean_up();
        
        void draw_frame();
        
        const VkDevice get_device() const {return m_device;}
        
    private:
        VkInstance m_instance;
        VkSurfaceKHR m_surface;
        evPhysicalDevice ev_physical_device;
        VkDevice m_device;
        
        VkQueue m_graphics_queue;
        VkQueue m_present_queue;
        
        VulkanSwapchain m_vulkan_swapchain;
        Pipeline m_pipeline;
        
        VkCommandPool m_command_pool;
        std::vector<VkCommandBuffer> m_command_buffers;
        
        VkBuffer m_vertex_buffer;
        VkDeviceMemory m_vertex_buffer_memory;
        
        VkBuffer m_index_buffer;
        VkDeviceMemory m_index_buffer_memory;
        
        std::vector<VkSemaphore> m_image_available_semaphores;
        std::vector<VkSemaphore> m_render_finished_semaphores;
        std::vector<VkFence> m_in_flight_fences;
        
        const int MAX_FRAMES_IN_FLIGHT = 2;
        uint32_t m_current_frame = 0;
        
        void create_instance();
        void create_surface(GLFWwindow* window);
        void create_logical_device();
        
        void create_command_pool();
        void create_command_buffer();
        void record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index);
        
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        
        void create_vertex_buffer();
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        void create_index_buffer();
        
        void create_sync_objects();
        
        void transition_image_layout(
            VkCommandBuffer command_buffer,
            uint32_t image_index,
            VkImageLayout old_layout,
            VkImageLayout new_layout,
            VkAccessFlags2 src_access_mask,
            VkAccessFlags2 dst_access_mask,
            VkPipelineStageFlags2 src_stage_mask,
            VkPipelineStageFlags2 dst_stage_mask
                                     );
    };
}
