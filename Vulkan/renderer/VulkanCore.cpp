#include "VulkanCore.h"
#include <set>
#include "../shapes/Vertex.h"

namespace evoke::vulkan {
    void VulkanCore::init_vulkan(GLFWwindow *window){
        create_instance();
        create_surface(window);
        pick_physical_device();
        m_indices = find_queue_families(m_physical_device, m_surface);
        create_logical_device();
        
        create_command_pool();
        create_vertex_buffer();
        create_index_buffer();
        create_sync_objects();
        create_command_buffer();
        
        m_swapchain_support = query_swapchain_support(m_physical_device);
        m_vulkan_swapchain.create_swapchain(m_swapchain_support, m_surface, m_indices, m_device);
        m_pipeline.create_pipeline(m_device, m_vulkan_swapchain);
    }
    
    void VulkanCore::clean_up(){
        if (m_device != VK_NULL_HANDLE) {
                vkDeviceWaitIdle(m_device);
            }
        
        utils::Logger::info("Cleaning up semaphores and fences!");
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                vkDestroySemaphore(m_device, m_render_finished_semaphores[i], nullptr);
                vkDestroySemaphore(m_device, m_image_available_semaphores[i], nullptr);
                vkDestroyFence(m_device, m_in_flight_fences[i], nullptr);
            }
        utils::Logger::info("Semaphores and fences cleaned up successfully!");
        
        utils::Logger::info("Cleaning up command pool!");
        vkDestroyCommandPool(m_device, m_command_pool, nullptr);
        utils::Logger::info("Command pool cleaned up successfully!");
        
        m_pipeline.clean_up(m_device);
        
        m_vulkan_swapchain.clean_up(m_device);
        
        vkDestroyBuffer(m_device, m_index_buffer, nullptr);
        vkFreeMemory(m_device, m_index_buffer_memory, nullptr);
        
        vkDestroyBuffer(m_device, m_vertex_buffer, nullptr);
        vkFreeMemory(m_device, m_vertex_buffer_memory, nullptr);
        
        utils::Logger::info("Cleaning up logical device!");
        vkDestroyDevice(m_device, nullptr);
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
    
    void VulkanCore::pick_physical_device(){
        utils::Logger::info("Selecting GPU!");
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);
        
        if (device_count == 0){
            utils::Logger::error("Failed to find suitable GPUs!");
        }
        
        std::vector<VkPhysicalDevice> physical_devices(device_count);
        vkEnumeratePhysicalDevices(m_instance, &device_count, physical_devices.data());
        
        for (const auto& physical_device : physical_devices) {
            if(is_device_suitable(physical_device)){
                m_physical_device = physical_device;
                break;
            }
        }
        
        if (m_physical_device == VK_NULL_HANDLE) {
            utils::Logger::error("Failed to find a suitable GPU!");
        }
        
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(m_physical_device, &deviceProperties);
        
        utils::Logger::info("Device Name: ", deviceProperties.deviceName);
        
        utils::Logger::info("GPU selected successfully!");
    }
    
    void VulkanCore::create_command_pool(){
        VkCommandPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pool_info.queueFamilyIndex = m_indices.graphics_family.value();
        
        if (vkCreateCommandPool(m_device, &pool_info, nullptr, &m_command_pool) != VK_SUCCESS) {
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

        if (vkAllocateCommandBuffers(m_device, &allocate_info, m_command_buffers.data()) != VK_SUCCESS) {
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
        color_attachment_info.imageView = m_vulkan_swapchain.get_vulkan_swapchain_image_views()[image_index];
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
        rendering_info.renderArea = { .offset = { 0, 0 }, .extent = m_vulkan_swapchain.get_vulkan_swapchain_extent() };
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
        viewport.width = static_cast<float>(m_vulkan_swapchain.get_vulkan_swapchain_extent().width);
        viewport.height = static_cast<float>(m_vulkan_swapchain.get_vulkan_swapchain_extent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(command_buffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = m_vulkan_swapchain.get_vulkan_swapchain_extent();
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
        vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, vertices.data(), (size_t) bufferSize);
        vkUnmapMemory(m_device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertex_buffer, m_vertex_buffer_memory);
        
        copyBuffer(stagingBuffer, m_vertex_buffer, bufferSize);

        vkDestroyBuffer(m_device, stagingBuffer, nullptr);
        vkFreeMemory(m_device, stagingBufferMemory, nullptr);
    }
    
    uint32_t VulkanCore::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_physical_device, &memProperties);

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
        vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), (size_t) bufferSize);
        vkUnmapMemory(m_device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_index_buffer, m_index_buffer_memory);

        copyBuffer(stagingBuffer, m_index_buffer, bufferSize);

        vkDestroyBuffer(m_device, stagingBuffer, nullptr);
        vkFreeMemory(m_device, stagingBufferMemory, nullptr);
    }
    
    void VulkanCore::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
    }
    
    void VulkanCore::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_command_pool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

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

        vkQueueSubmit(m_graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_graphics_queue);

        vkFreeCommandBuffers(m_device, m_command_pool, 1, &commandBuffer);
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
        barrier.image = m_vulkan_swapchain.get_vulkan_swapchain_images()[image_index];
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
            if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_image_available_semaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_render_finished_semaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_device, &fence_info, nullptr, &m_in_flight_fences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }
    
    void VulkanCore::draw_frame(){
        vkWaitForFences(m_device, 1, &m_in_flight_fences[m_current_frame], VK_TRUE, UINT64_MAX);
        vkResetFences(m_device, 1, &m_in_flight_fences[m_current_frame]);
        
        uint32_t image_index;
        vkAcquireNextImageKHR(m_device, m_vulkan_swapchain.get_swapchain(), UINT64_MAX, m_image_available_semaphores[m_current_frame], VK_NULL_HANDLE, &image_index);
        
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
        
        if (vkQueueSubmit(m_graphics_queue, 1, &submit_info, m_in_flight_fences[m_current_frame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }
        
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signal_semaphores;
        
        VkSwapchainKHR swapchains[] = {m_vulkan_swapchain.get_swapchain()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &image_index;
                
        vkQueuePresentKHR(m_present_queue, &presentInfo);
        
        m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
    
    bool VulkanCore::is_device_suitable(VkPhysicalDevice physical_device){
        QueueFamilyIndices indices = find_queue_families(physical_device, m_surface);
        
        bool extensions_supported = does_device_support_extensions(physical_device);
        
        bool swapchain_adequate = false;
        if (extensions_supported) {
            SwapchainSupportDetails swapchain_support = query_swapchain_support(physical_device);
            swapchain_adequate = !swapchain_support.formats.empty() && !swapchain_support.present_modes.empty();
        }

        return indices.is_complete() && extensions_supported && swapchain_adequate;
    }
    
    bool VulkanCore::does_device_support_extensions(VkPhysicalDevice physical_device) {
        uint32_t extension_count;
        vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);

        std::vector<VkExtensionProperties> available_extensions(extension_count);
        vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, available_extensions.data());

        std::set<std::string> requiredExtensions(m_device_extensions.begin(), m_device_extensions.end());

        for (const auto& extension : available_extensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }
    
    QueueFamilyIndices VulkanCore::find_queue_families(VkPhysicalDevice physical_device, VkSurfaceKHR surface){
        QueueFamilyIndices indices;

            uint32_t queue_family_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

            std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

            int i = 0;
            for (const auto& queue_family : queue_families) {
                if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    indices.graphics_family = i;
                }
                
                VkBool32 present_support = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);
                
                if (present_support) {
                    indices.present_family = i;
                }
                
                if (indices.is_complete()) {
                        break;
                    }

                i++;
            }
        
        return indices;
    }
    
    SwapchainSupportDetails VulkanCore::query_swapchain_support(VkPhysicalDevice physical_device){
        SwapchainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, m_surface, &details.capabilities);
        
        uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, m_surface, &format_count, nullptr);

        if (format_count != 0) {
            details.formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, m_surface, &format_count, details.formats.data());
        }
        
        uint32_t present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, m_surface, &present_mode_count, nullptr);

        if (present_mode_count != 0) {
            details.present_modes.resize(present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, m_surface, &present_mode_count, details.present_modes.data());
        }
        
        return details;
    }
    
    void VulkanCore::create_logical_device(){
        utils::Logger::info("Creating logical device!");
        QueueFamilyIndices indices = find_queue_families(m_physical_device, m_surface);

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
        std::set<uint32_t> unique_queue_families = {indices.graphics_family.value(), indices.present_family.value()};
        
        float queue_priority = 1.0f;
        for (uint32_t queue_family : unique_queue_families) {
            VkDeviceQueueCreateInfo queue_create_info{};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = queue_family;
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &queue_priority;
            queue_create_infos.push_back(queue_create_info);
        }
        
        VkPhysicalDeviceFeatures device_features{};
        VkDeviceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
        create_info.pQueueCreateInfos = queue_create_infos.data();
        create_info.pEnabledFeatures = &device_features;
        create_info.enabledExtensionCount = static_cast<uint32_t>(m_device_extensions.size());
        create_info.ppEnabledExtensionNames = m_device_extensions.data();
        
        if(vkCreateDevice(m_physical_device, &create_info, nullptr, &m_device) != VK_SUCCESS){
            utils::Logger::error("Failed to create logical device!");
        }
        
        vkGetDeviceQueue(m_device, indices.graphics_family.value(), 0, &m_graphics_queue);
        vkGetDeviceQueue(m_device, indices.present_family.value(), 0, &m_present_queue);
        
        utils::Logger::info("Logical device created successfully!");
    }
}
