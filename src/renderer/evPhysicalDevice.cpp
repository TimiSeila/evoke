#include "evPhysicalDevice.h"
#include "../utils/Logger.h"

void evPhysicalDevice::init(VkInstance instance, VkSurfaceKHR surface){
    pick_physical_device(instance, surface);
}

void evPhysicalDevice::pick_physical_device(VkInstance instance, VkSurfaceKHR surface){
    //Check if physical devices exist
    uint32_t physical_device_count = 0;
    vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);
    
    if(physical_device_count == 0){
        evoke::utils::Logger::error("No physical devices found");
    }
    
    //Store all found physical devices
    std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
    vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data());
    
    //Check which device is suitable
    for (const auto& physical_device : physical_devices) {
        if(is_device_suitable(physical_device, surface)){
            //Store handle in wrapper struct
            physical_device_info.handle = physical_device;
            
            //Query and store physical device properties and features in wrapper struct
            vkGetPhysicalDeviceProperties(physical_device, &physical_device_info.properties);
            vkGetPhysicalDeviceFeatures(physical_device, &physical_device_info.features);
            
            //Query and store queue family indices in wrapper struct
            physical_device_info.queue_family_indices = query_queue_families(physical_device, surface);
            
            //Query and store all supported extensions
            physical_device_info.extensions_info = query_extension_support(physical_device);
            
            //Query and store swapchain support info in wrapper struct
            physical_device_info.swapchain_support = query_swapchain_support(physical_device, surface);
            break;
        }
    }
    
    if (physical_device_info.handle == VK_NULL_HANDLE) {
        evoke::utils::Logger::error("No suitable physical devices found");
    }
}

bool evPhysicalDevice::is_device_suitable(VkPhysicalDevice physical_device, VkSurfaceKHR surface){
    QueueFamilyIndices indices = query_queue_families(physical_device, surface);
    
    ExtensionSupportInfo extension_info = query_extension_support(physical_device);
    
    SwapchainSupportInfo swapchain_info = query_swapchain_support(physical_device, surface);
    
    return indices.is_complete() && extension_info.is_adequate() && swapchain_info.is_adequate();
}

QueueFamilyIndices evPhysicalDevice::query_queue_families(VkPhysicalDevice physical_device, VkSurfaceKHR surface){
    QueueFamilyIndices indices;
    
    //Check if queue families exist
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

    //Store all found queue families
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());
    
    //Check if graphic and present families are supported and store indices
    uint32_t i = 0;
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

SwapchainSupportInfo evPhysicalDevice::query_swapchain_support(VkPhysicalDevice physical_device, VkSurfaceKHR surface){
    SwapchainSupportInfo swapchain_info;

    //Store surface capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &swapchain_info.capabilities);
    
    //Check if surface formats exist
    uint32_t surface_format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, nullptr);

    //Store surface formats
    if (surface_format_count != 0) {
        swapchain_info.surface_formats.resize(surface_format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, swapchain_info.surface_formats.data());
    }
    
    //Check if present modes exist
    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);

    //Store present modes
    if (present_mode_count != 0) {
        swapchain_info.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, swapchain_info.present_modes.data());
    }
    
    return swapchain_info;
}

ExtensionSupportInfo evPhysicalDevice::query_extension_support(VkPhysicalDevice physical_device){
    ExtensionSupportInfo info;

    //Check if extensions exist
    uint32_t count;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, nullptr);

    //Store extensions
    std::vector<VkExtensionProperties> available_extensions(count);
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, available_extensions.data());

    
    //Fill extensions based on which required ones are available
    for (const auto& required : info.required) {
        bool found = false;
        for (const auto& available : available_extensions) {
            if (strcmp(required, available.extensionName) == 0) {
                found = true;
                break;
            }
        }

        if (found) {
            info.extensions.push_back(required);
        }
    }

    return info;
}