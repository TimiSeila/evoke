#include "VulkanCore.h"
#include <set>
#include "../shapes/Vertex.h"

namespace evoke::vulkan {
    void VulkanCore::init_vulkan(GLFWwindow *window){
        create_instance();
        create_surface(window);
        ev_physical_device.init(m_instance, m_surface);
        ev_device.init(ev_physical_device);
        
        create_command_pool();
        create_vertex_buffer();
        create_index_buffer();
        create_sync_objects();
        create_command_buffer();
        
        ev_swapchain.init(ev_device.get().handle, ev_physical_device, m_surface, window);
        m_pipeline.create_pipeline(ev_device.get().handle, ev_swapchain.get().surface_format);
    }
    
    void VulkanCore::clean_up(){
        if (ev_device.get().handle != VK_NULL_HANDLE) {
                vkDeviceWaitIdle(ev_device.get().handle);
            }
        
        utils::Logger::info("Cleaning up semaphores and fences!");
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                vkDestroySemaphore(ev_device.get().handle, m_render_finished_semaphores[i], nullptr);
                vkDestroySemaphore(ev_device.get().handle, m_image_available_semaphores[i], nullptr);
                vkDestroyFence(ev_device.get().handle, m_in_flight_fences[i], nullptr);
            }
        utils::Logger::info("Semaphores and fences cleaned up successfully!");
        
        utils::Logger::info("Cleaning up command pool!");
        vkDestroyCommandPool(ev_device.get().handle, m_command_pool, nullptr);
        utils::Logger::info("Command pool cleaned up successfully!");
        
        m_pipeline.clean_up(ev_device.get().handle);
        
        ev_swapchain.clean_up(ev_device.get().handle);
        
        vkDestroyBuffer(ev_device.get().handle, m_index_buffer, nullptr);
        vkFreeMemory(ev_device.get().handle, m_index_buffer_memory, nullptr);
        
        vkDestroyBuffer(ev_device.get().handle, m_vertex_buffer, nullptr);
        vkFreeMemory(ev_device.get().handle, m_vertex_buffer_memory, nullptr);
        
        utils::Logger::info("Cleaning up logical device!");
        ev_device.clean_up();
        utils::Logger::info("Logical device cleaned up successfully!");
        
        utils::Logger::info("Cleaning up surface!");
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        utils::Logger::info("Surface cleaned up successfully!");
        
        utils::Logger::info("Cleaning up vulkan instance!");
        vkDestroyInstance(m_instance, nullptr);
        utils::Logger::info("Vulkan instance cleaned up successfully!");
    }
    
    void VulkanCore::create_instance(){
        utils::Logger::info("Creating vulkan instance!");
        
        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Evoke";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "Evoke";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_4;
        
        VkInstanceCreateInfo instance_create_info{};
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        instance_create_info.pApplicationInfo = &app_info;
        instance_create_info.enabledLayerCount = 0;
        
        uint32_t glfw_extension_count = 0;
        const char** glfw_extensions;
        
        glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

        std::vector<const char*> required_extensions;

        for(uint32_t i = 0; i < glfw_extension_count; i++) {
            required_extensions.emplace_back(glfw_extensions[i]);
        }
        required_extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        
        instance_create_info.enabledExtensionCount = (uint32_t) required_extensions.size();
        instance_create_info.ppEnabledExtensionNames = required_extensions.data();
        
        if(vkCreateInstance(&instance_create_info, nullptr, &m_instance) != VK_SUCCESS){
            evoke::utils::Logger::error("Failed to create instance!");
        }
        
        utils::Logger::info("Vulkan instance created successfully!");
    }
    
    void VulkanCore::create_surface(GLFWwindow *window){
        utils::Logger::info("Creating surface!");
        if (glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface) != VK_SUCCESS) {
            utils::Logger::error("Failed to create surface!");
        }
        utils::Logger::info("Surface created successfully!");
    }
    
    void VulkanCore::create_command_pool(){
        VkCommandPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pool_info.queueFamilyIndex = ev_physical_device.get().queue_family_indices.graphics_family.value();
        
        if (vkCreateCommandPool(ev_device.get().handle, &pool_info, nullptr, &m_command_pool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }
    
    void VulkanCore::create_command_buffer(){
        m_command_buffers.resize(MAX_FRAMES_IN_FLIGHT);
        
        VkCommandBufferAllocateInfo allocate_info{};
        allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocate_info.commandPool = m_command_pool;
        allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocate_info.commandBufferCount = (uint32_t) m_command_buffers.size();

        if (vkAllocateCommandBuffers(ev_device.get().handle, &allocate_info, m_command_buffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }
    
    void VulkanCore::record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index){
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = 0; // Optional
        begin_info.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
        
        transition_image_layout(
            command_buffer,
            image_index,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            0,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
        );
        
        VkClearValue clear_color = { .color = { .float32 = { 0.5f, 0.9f, 0.6f, 1.0f } } };

        VkRenderingAttachmentInfo color_attachment_info{};
        color_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        color_attachment_info.pNext = NULL;
        color_attachment_info.imageView = ev_swapchain.get().image_views[image_index];
        color_attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color_attachment_info.resolveMode = VK_RESOLVE_MODE_NONE;
        color_attachment_info.resolveImageView = VK_NULL_HANDLE;
        color_attachment_info.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment_info.clearValue = clear_color;
        
        VkRenderingInfo rendering_info{};
        rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        rendering_info.pNext = NULL;
        rendering_info.flags = 0;
        rendering_info.renderArea = { .offset = { 0, 0 }, .extent = ev_swapchain.get().extent };
        rendering_info.layerCount = 1;
        rendering_info.viewMask = 0;
        rendering_info.colorAttachmentCount = 1;
        rendering_info.pColorAttachments = &color_attachment_info;
        rendering_info.pDepthAttachment = NULL;
        rendering_info.pStencilAttachment = NULL;
        
        vkCmdBeginRendering(command_buffer, &rendering_info);
        
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.get_graphics_pipeline());
        
        VkBuffer vertexBuffers[] = {m_vertex_buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(command_buffer, m_index_buffer, 0, VK_INDEX_TYPE_UINT16);
        
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(ev_swapchain.get().extent.width);
        viewport.height = static_cast<float>(ev_swapchain.get().extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(command_buffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = ev_swapchain.get().extent;
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);
        
        vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        
        vkCmdEndRendering(command_buffer);
        
        transition_image_layout(
            command_buffer,
            image_index,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            0,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT
        );
        
        vkEndCommandBuffer(command_buffer);
    }
    
    void VulkanCore::create_vertex_buffer(){
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(ev_device.get().handle, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, vertices.data(), (size_t) bufferSize);
        vkUnmapMemory(ev_device.get().handle, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertex_buffer, m_vertex_buffer_memory);
        
        copyBuffer(stagingBuffer, m_vertex_buffer, bufferSize);

        vkDestroyBuffer(ev_device.get().handle, stagingBuffer, nullptr);
        vkFreeMemory(ev_device.get().handle, stagingBufferMemory, nullptr);
    }
    
    uint32_t VulkanCore::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(ev_physical_device.get().handle, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }
    
    void VulkanCore::create_index_buffer() {
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(ev_device.get().handle, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), (size_t) bufferSize);
        vkUnmapMemory(ev_device.get().handle, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_index_buffer, m_index_buffer_memory);

        copyBuffer(stagingBuffer, m_index_buffer, bufferSize);

        vkDestroyBuffer(ev_device.get().handle, stagingBuffer, nullptr);
        vkFreeMemory(ev_device.get().handle, stagingBufferMemory, nullptr);
    }
    
    void VulkanCore::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(ev_device.get().handle, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(ev_device.get().handle, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(ev_device.get().handle, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(ev_device.get().handle, buffer, bufferMemory, 0);
    }
    
    void VulkanCore::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_command_pool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(ev_device.get().handle, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

            VkBufferCopy copyRegion{};
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(ev_device.get().graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(ev_device.get().graphics_queue);

        vkFreeCommandBuffers(ev_device.get().handle, m_command_pool, 1, &commandBuffer);
    }
    
    void VulkanCore::transition_image_layout(VkCommandBuffer command_buffer, uint32_t image_index, VkImageLayout old_layout, VkImageLayout new_layout, VkAccessFlags2 src_access_mask, VkAccessFlags2 dst_access_mask, VkPipelineStageFlags2 src_stage_mask, VkPipelineStageFlags2 dst_stage_mask){
        VkImageMemoryBarrier2 barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        barrier.srcStageMask = src_stage_mask;
        barrier.srcAccessMask = src_access_mask;
        barrier.dstStageMask = dst_stage_mask;
        barrier.dstAccessMask = dst_access_mask;
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = ev_swapchain.get().images[image_index];
        barrier.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        };
        
        VkDependencyInfo dependency_info{};
        dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependency_info.dependencyFlags = {};
        dependency_info.imageMemoryBarrierCount = 1;
        dependency_info.pImageMemoryBarriers = &barrier;
            
        vkCmdPipelineBarrier2(command_buffer, &dependency_info);
    }
    
    void VulkanCore::create_sync_objects(){
        m_image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
        
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        
        VkFenceCreateInfo fence_info{};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(ev_device.get().handle, &semaphoreInfo, nullptr, &m_image_available_semaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(ev_device.get().handle, &semaphoreInfo, nullptr, &m_render_finished_semaphores[i]) != VK_SUCCESS ||
                vkCreateFence(ev_device.get().handle, &fence_info, nullptr, &m_in_flight_fences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }
    
    void VulkanCore::draw_frame(){
        vkWaitForFences(ev_device.get().handle, 1, &m_in_flight_fences[m_current_frame], VK_TRUE, UINT64_MAX);
        vkResetFences(ev_device.get().handle, 1, &m_in_flight_fences[m_current_frame]);
        
        uint32_t image_index;
        vkAcquireNextImageKHR(ev_device.get().handle, ev_swapchain.get().handle, UINT64_MAX, m_image_available_semaphores[m_current_frame], VK_NULL_HANDLE, &image_index);
        
        vkResetCommandBuffer(m_command_buffers[m_current_frame], 0);
        record_command_buffer(m_command_buffers[m_current_frame], image_index);
        
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pCommandBuffers = &m_command_buffers[m_current_frame];

        VkSemaphore wait_semaphores[] = {m_image_available_semaphores[m_current_frame]};
        VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = wait_semaphores;
        submit_info.pWaitDstStageMask = wait_stages;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_command_buffers[m_current_frame];
        
        VkSemaphore signal_semaphores[] = {m_render_finished_semaphores[m_current_frame]};
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = signal_semaphores;
        
        if (vkQueueSubmit(ev_device.get().graphics_queue, 1, &submit_info, m_in_flight_fences[m_current_frame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }
        
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signal_semaphores;
        
        VkSwapchainKHR swapchains[] = {ev_swapchain.get().handle};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &image_index;
                
        vkQueuePresentKHR(ev_device.get().presentation_queue, &presentInfo);
        
        m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
}