#include "VulkanSwapchain.h"
#include <iostream>

namespace evoke::vulkan {
    void VulkanSwapchain::create_swapchain(const SwapchainSupportDetails& swapchain_support, VkSurfaceKHR surface, QueueFamilyIndices indices, VkDevice device){
        utils::Logger::info("Creating swapchain!");
        
        VkSurfaceFormatKHR surface_format = choose_swapchain_surface_format(swapchain_support.formats);
        VkPresentModeKHR present_mode = choose_swap_present_mode(swapchain_support.present_modes);
        VkExtent2D extent = choose_swap_extent(swapchain_support.capabilities);
        
        uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
        
        if (swapchain_support.capabilities.maxImageCount > 0 && image_count > swapchain_support.capabilities.maxImageCount) {
            image_count = swapchain_support.capabilities.maxImageCount;
        }
        
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = image_count;
        createInfo.imageFormat = surface_format.format;
        createInfo.imageColorSpace = surface_format.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        
        uint32_t queue_family_indices[] = {indices.graphics_family.value(), indices.present_family.value()};

        if (indices.graphics_family != indices.present_family) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queue_family_indices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }
        
        createInfo.preTransform = swapchain_support.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = present_mode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;
        
        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }
        
        utils::Logger::info("Swapchain created successfully!");
        
        vkGetSwapchainImagesKHR(device, m_swapchain, &image_count, nullptr);
        m_swapchain_images.resize(image_count);
        vkGetSwapchainImagesKHR(device, m_swapchain, &image_count, m_swapchain_images.data());
        
        m_swapchain_image_format = surface_format.format;
        m_swapchain_extent = extent;
        
        create_image_views(device);
    }
    
    void VulkanSwapchain::clean_up(VkDevice device){
        utils::Logger::info("Cleaning up swapchain!");
        for (auto image_view : m_swapchain_image_views) {
                vkDestroyImageView(device, image_view, nullptr);
            }
        
        vkDestroySwapchainKHR(device, m_swapchain, nullptr);
        utils::Logger::info("Swapchain cleaned up successfully!");
    }
    
    VkSurfaceFormatKHR VulkanSwapchain::choose_swapchain_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats) {
        for (const auto& available_format : available_formats) {
            if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return available_format;
            }
        }

        return available_formats[0];
    }
    
    VkPresentModeKHR VulkanSwapchain::choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes) {
        for (const auto& available_present_mode : available_present_modes) {
            if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return available_present_mode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }
    
    VkExtent2D VulkanSwapchain::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width = 800;
            int height = 600;
            //glfwGetFramebufferSize(/*window*/, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }
    
    void VulkanSwapchain::create_image_views(VkDevice device){
        m_swapchain_image_views.resize(m_swapchain_images.size());
        
        for (size_t i = 0; i < m_swapchain_images.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_swapchain_images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_swapchain_image_format;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;
            
            if (vkCreateImageView(device, &createInfo, nullptr, &m_swapchain_image_views[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image views!");
            }
        }
    }
}
