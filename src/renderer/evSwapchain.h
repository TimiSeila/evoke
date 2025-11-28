#pragma once

#include <vulkan/vulkan.h>
#include "evPhysicalDevice.h"
#include "../utils/Logger.h"
#include "../core/Window.h"

//Wrapper for swapchain
struct evSwapchainInfo{
    VkSwapchainKHR handle;
    std::vector<VkImage> images;
    std::vector<VkImageView> image_views;
    VkSurfaceFormatKHR surface_format;
    VkPresentModeKHR present_mode;
    VkExtent2D extent;
};

class evSwapchain{
public:
    void init(VkDevice device, evPhysicalDevice& physical_device, VkSurfaceKHR surface, GLFWwindow* window);
    void clean_up(VkDevice device);

    const evSwapchainInfo& get() const { return swapchain_info;}

private:
    evSwapchainInfo swapchain_info;

    void create_swapchain(VkDevice device, evPhysicalDevice& physical_device, VkSurfaceKHR surface, GLFWwindow* window);
    VkSurfaceFormatKHR choose_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
    VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);
    VkExtent2D choose_extent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);
    void create_image_views(VkDevice device, const std::vector<VkImage>& images);
};