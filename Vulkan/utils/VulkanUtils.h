#pragma once
#include "VulkanUtils.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <fstream>
#include "Logger.h"
#include <iostream>

namespace evoke::vulkan {
    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };
    
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;

        bool is_complete() const {
            return graphics_family.has_value() && present_family.has_value();
        }
    };
    
    static std::vector<char> read_file(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            evoke::utils::Logger::error("Couldn't open file!");
        }
        
        size_t file_size = (size_t) file.tellg();
        std::vector<char> buffer(file_size);
        
        file.seekg(0);
        file.read(buffer.data(), file_size);
        file.close();
        
        evoke::utils::Logger::info("File read successfully!");
        
        return buffer;
    }
}
