#pragma once
#include "../renderer/VulkanCore.h"
#include "Window.h"

namespace evoke::core {
    class Application {
    public:
        void run();
        
    private:
        Window m_window;
        vulkan::VulkanCore m_vulkan_core;
        
        
        void init_app();
        void main_loop();
        void clean_up();
    };
}
