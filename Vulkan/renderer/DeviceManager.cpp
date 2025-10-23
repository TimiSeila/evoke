#include "DeviceManager.h"

namespace vbs_engine::renderer {
void DeviceManager::initDeviceManager(){
    std::cout << "Initializing Device Manager!\n";
    DeviceManager::create_instance();
    DeviceManager::pick_physical_device();
    DeviceManager::create_device();
    std::cout << "Device Manager initialization successfull!\n";
}

VkInstance DeviceManager::get_vk_instance(){
    return m_vk_instance;
}

void DeviceManager::create_instance(){
    std::cout << "Creating vulkan instance!\n";
    //Struct providing information to the driver for optimization
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "VBS 6";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "vbs_engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_4;
    
    //Struct that tells vulkan which extensions and validation layers to use
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    
    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
    
    if (vkCreateInstance(&createInfo, nullptr, &m_vk_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
    std::cout << "Vulkan instance created successfully!\n";
}

void DeviceManager::pick_physical_device(){
    std::cout << "Picking physical device!\n";
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_vk_instance, &deviceCount, nullptr);
    
    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }
    
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_vk_instance, &deviceCount, devices.data());
    
    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            m_vk_physical_device = device;
            break;
        }
    }
    
    if (m_vk_physical_device == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
    
    // Get device properties for printing
    VkPhysicalDeviceProperties vk_physical_device_props;
    vkGetPhysicalDeviceProperties(m_vk_physical_device, &vk_physical_device_props);
    
    std::cout << "Physical device picked successfully!: " << vk_physical_device_props.deviceName << "\n";
}

void DeviceManager::create_device(){
    std::cout << "Creating logical device!\n";
    //        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    //
    //        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    //        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    //
    //        float queuePriority = 1.0f;
    //        for (uint32_t queueFamily : uniqueQueueFamilies) {
    //            VkDeviceQueueCreateInfo queueCreateInfo{};
    //            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    //            queueCreateInfo.queueFamilyIndex = queueFamily;
    //            queueCreateInfo.queueCount = 1;
    //            queueCreateInfo.pQueuePriorities = &queuePriority;
    //            queueCreateInfos.push_back(queueCreateInfo);
    //        }
    //
    //        VkPhysicalDeviceFeatures deviceFeatures{};
    //
    //        VkDeviceCreateInfo createInfo{};
    //        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    //
    //        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    //        createInfo.pQueueCreateInfos = queueCreateInfos.data();
    //
    //        createInfo.pEnabledFeatures = &deviceFeatures;
    //
    //        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    //        createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    //
    //        if (enableValidationLayers) {
    //            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    //            createInfo.ppEnabledLayerNames = validationLayers.data();
    //        } else {
    //            createInfo.enabledLayerCount = 0;
    //        }
    //
    //        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
    //            throw std::runtime_error("failed to create logical device!");
    //        }
    //
    //        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    //        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    std::cout << "Logical device created successfully!\n";
}

std::vector<const char*> DeviceManager::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    
    extensions.push_back("VK_KHR_portability_enumeration");
    
    return extensions;
}

bool DeviceManager::isDeviceSuitable(VkPhysicalDevice device){
    return true;
}
}
