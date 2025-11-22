#pragma once
#define GLFW_EXPOSE_NATIVE_COCOA
#define VK_USE_PLATFORM_METAL_EXT
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vector>

#include "../utils/VulkanUtils.h"
#include "../core/Window.h"

namespace evoke::vulkan {
    class VulkanSwapchain{
    public:
        void create_swapchain(const SwapchainSupportDetails& swapchain_support, VkSurfaceKHR surface, QueueFamilyIndices indices, VkDevice device);
        
        VkSwapchainKHR get_swapchain() const {return m_swapchain;}
        std::vector<VkImage> get_vulkan_swapchain_images() const {return m_swapchain_images;};
        std::vector<VkImageView> get_vulkan_swapchain_image_views() const {return m_swapchain_image_views;};
        VkExtent2D get_vulkan_swapchain_extent() const { return m_swapchain_extent;}
        VkFormat& get_vulkan_swapchain_format() { return m_swapchain_image_format;}
        
        void clean_up(VkDevice device);
    private:
        VkSwapchainKHR m_swapchain;
        std::vector<VkImage> m_swapchain_images;
        VkFormat m_swapchain_image_format;
        VkExtent2D m_swapchain_extent;
        
        std::vector<VkImageView> m_swapchain_image_views;
        
        VkSurfaceFormatKHR choose_swapchain_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
        VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);
        VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);
        
        void create_image_views(VkDevice device);
    };
}
