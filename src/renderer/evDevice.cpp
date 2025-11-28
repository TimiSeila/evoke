#include "evDevice.h"
#include <vector>
#include <set>

void evDevice::init(evPhysicalDevice& physical_device){
    create_device(physical_device);
}

void evDevice::create_device(evPhysicalDevice& physical_device){
    //Create infos for queues
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    
    //Unique queue families stored in physical device wrapper
    std::set<uint32_t> unique_queue_families = {physical_device.get().queue_family_indices.graphics_family.value(), physical_device.get().queue_family_indices.present_family.value()};
    
    float queue_priority = 1.0f;
    for (uint32_t queue_family : unique_queue_families) {
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        
        queue_create_infos.push_back(queue_create_info);
    }
    
    //Logical device create info
    VkPhysicalDeviceFeatures device_features{};
    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    create_info.pQueueCreateInfos = queue_create_infos.data();
    create_info.pEnabledFeatures = &device_features;
    create_info.enabledExtensionCount = static_cast<uint32_t>(physical_device.get().extensions_info.extensions.size());
    create_info.ppEnabledExtensionNames = physical_device.get().extensions_info.extensions.data();
    
    //Create logical device and store it in wrapper struct
    if(vkCreateDevice(physical_device.get().handle, &create_info, nullptr, &device_info.handle) != VK_SUCCESS){
        evoke::utils::Logger::error("Failed to create logical device");
    };
                   
    //Get queue handles and store them in wrapper struct
    vkGetDeviceQueue(device_info.handle, physical_device.get().queue_family_indices.graphics_family.value(), 0, &device_info.graphics_queue);
    vkGetDeviceQueue(device_info.handle, physical_device.get().queue_family_indices.present_family.value(), 0, &device_info.presentation_queue);
    
}

void evDevice::clean_up() {
    if (device_info.handle != VK_NULL_HANDLE) {
        vkDestroyDevice(device_info.handle, nullptr);
        device_info.handle = VK_NULL_HANDLE;
    }
}