#pragma once

#include "vulkan/vulkan.h"
#include <optional>
#include <vector>
#include <algorithm>
#include <cstring>

//Wrapper for queue families
struct QueueFamilyIndices {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    bool is_complete() const {
        return graphics_family.has_value() && present_family.has_value();
    }
};

//Wrapper for swapchain support
struct SwapchainSupportInfo {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> surface_formats;
    std::vector<VkPresentModeKHR> present_modes;
    
    bool is_adequate() const {
        return !surface_formats.empty() && !present_modes.empty();
    }
};

//Wrapper for supported extensions
struct ExtensionSupportInfo {
    std::vector<const char*> required = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    std::vector<const char*> extensions;
    
    bool is_adequate() const {
        for (const char* required_extension : required) {
            if (std::find(extensions.begin(), extensions.end(), required_extension) == extensions.end())
                return false;
        }
        return true;
    }
};

//Wrapper for physical device
struct evPhysicalDeviceInfo {
    VkPhysicalDevice handle;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    QueueFamilyIndices queue_family_indices;
    SwapchainSupportInfo swapchain_support;
    ExtensionSupportInfo extensions_info;
};

class evPhysicalDevice {
public:
    void init(VkInstance instance, VkSurfaceKHR surface);
    
    const evPhysicalDeviceInfo& get() const { return physical_device_info; }
    
private:
    evPhysicalDeviceInfo physical_device_info = {};
    
    void pick_physical_device(VkInstance instance, VkSurfaceKHR surface);
    bool is_device_suitable(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
    QueueFamilyIndices query_queue_families(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
    SwapchainSupportInfo query_swapchain_support(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
    ExtensionSupportInfo query_extension_support(VkPhysicalDevice physical_device);
};