#include "evSwapchain.h"

void evSwapchain::init(VkDevice device, evPhysicalDevice& physical_device, VkSurfaceKHR surface, GLFWwindow* window){
    create_swapchain(device, physical_device, surface, window);
}

void evSwapchain::create_swapchain(VkDevice device, evPhysicalDevice& physical_device, VkSurfaceKHR surface, GLFWwindow* window){
    //Choose depending on swapchain support queried in physical device
    swapchain_info.surface_format = choose_surface_format(physical_device.get().swapchain_support.surface_formats);
    swapchain_info.present_mode = choose_present_mode(physical_device.get().swapchain_support.present_modes);
    swapchain_info.extent = choose_extent(physical_device.get().swapchain_support.capabilities, window);

    //Set image count to minimum + 1 if its not over max
    uint32_t image_count = physical_device.get().swapchain_support.capabilities.minImageCount + 1;

    if(physical_device.get().swapchain_support.capabilities.maxImageCount > 0 && image_count > physical_device.get().swapchain_support.capabilities.maxImageCount){
            image_count = physical_device.get().swapchain_support.capabilities.maxImageCount;
    }

    //Swapchain create info
    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = swapchain_info.surface_format.format;
    create_info.imageColorSpace = swapchain_info.surface_format.colorSpace;
    create_info.imageExtent = swapchain_info.extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    //Check if graphics and presentation family is the same
    uint32_t queue_family_indices[] = {
        physical_device.get().queue_family_indices.graphics_family.value(),
        physical_device.get().queue_family_indices.present_family.value()
    };

    if (physical_device.get().queue_family_indices.graphics_family != physical_device.get().queue_family_indices.present_family) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0; // Optional
        create_info.pQueueFamilyIndices = nullptr; // Optional
    }

    create_info.preTransform = physical_device.get().swapchain_support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = swapchain_info.present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    //Create swapchain and store it in wrapper struct
    if (vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain_info.handle) != VK_SUCCESS) {
        evoke::utils::Logger::error("Failed to create swapchain!");
    }

    //Get image handles from swapchain and store them in wrapper struct
    vkGetSwapchainImagesKHR(device, swapchain_info.handle, &image_count, nullptr);
    swapchain_info.images.resize(image_count);
    vkGetSwapchainImagesKHR(device, swapchain_info.handle, &image_count, swapchain_info.images.data());

    //Create image views from images
    create_image_views(device, swapchain_info.images);
}

VkSurfaceFormatKHR evSwapchain::choose_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats){
    //Loop through formats and find one that meets requirements
    for (const auto& available_format : available_formats) {
            if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return available_format;
            }
        }

    //If none found that meet requirements default to first in list
    return available_formats[0];
}
VkPresentModeKHR evSwapchain::choose_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes){
    //Loop through present modes and find one that meets requirements
    for (const auto& available_present_mode : available_present_modes) {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_mode;
        }
    }

    //If none found that meet requirements default to first fifo
    return VK_PRESENT_MODE_FIFO_KHR;
}
VkExtent2D evSwapchain::choose_extent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window){
    //TODO -- No idea
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actual_extent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actual_extent;
    }
}
void evSwapchain::create_image_views(VkDevice device, const std::vector<VkImage>& images){
    //Resize image views vector to image vector size
    swapchain_info.image_views.resize(swapchain_info.images.size());

    //Create image view info for each image
    for (size_t i = 0; i < swapchain_info.images.size(); i++) {
        VkImageViewCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = swapchain_info.images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = swapchain_info.surface_format.format;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;
            
        //Create image view and store it in wrapper struct
        if (vkCreateImageView(device, &create_info, nullptr, &swapchain_info.image_views[i]) != VK_SUCCESS) {
            evoke::utils::Logger::error("Failed to create image view!");
        }
    }
}

void evSwapchain::clean_up(VkDevice device){
    for (auto image_view : swapchain_info.image_views) {
                vkDestroyImageView(device, image_view, nullptr);
            }
        
        vkDestroySwapchainKHR(device, swapchain_info.handle, nullptr);
}