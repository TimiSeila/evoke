#pragma once

#include <vulkan/vulkan.h>
#include "evPhysicalDevice.h"
#include "../utils/Logger.h"

//Wrapper for device
struct evDeviceInfo {
    VkDevice handle;
    VkQueue graphics_queue;
    VkQueue presentation_queue;
};

class evDevice {
public:
    void init(evPhysicalDevice& physical_device);
    void clean_up();
    
    const evDeviceInfo& get() const { return device_info; }
private:
    evDeviceInfo device_info;
    
    void create_device(evPhysicalDevice& physical_device);
};