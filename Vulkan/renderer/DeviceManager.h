#pragma once
#define VK_USE_PLATFORM_METAL_EXT
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <iostream>

namespace vbs_engine::renderer {
class DeviceManager{
public:
    void initDeviceManager();
    
    VkInstance get_vk_instance();
    VkPhysicalDevice get_vk_physical_device();
    VkDevice get_vk_device();
private:
    VkInstance m_vk_instance;
    VkPhysicalDevice m_vk_physical_device = VK_NULL_HANDLE;
    VkDevice m_vk_device;
    
    void create_instance();
    void pick_physical_device();
    void create_device();
    std::vector<const char*> getRequiredExtensions();
    bool isDeviceSuitable(VkPhysicalDevice device);
};
}
