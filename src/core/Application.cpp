#include "Application.h"
#include <iostream>
#include "../utils/Logger.h"

namespace evoke::core {
    void Application::run() {
        init_app();
        main_loop();
        clean_up();
    }
    
    void Application::init_app() {
        evoke::utils::Logger::info("Initializing application!");
        
        m_window.init_window();
        m_vulkan_core.init_vulkan(m_window.get_glfw_window());
        
        evoke::utils::Logger::info("Application initialized successfully!");
    }
    
    int frame = 0;
    auto lastTime = std::chrono::high_resolution_clock::now();
    double fps = 0.0;
    
    void Application::main_loop() {
        while (!glfwWindowShouldClose(m_window.get_glfw_window())) {
            glfwPollEvents();
            m_vulkan_core.draw_frame();
            frame++;

            // Measure time
            auto currentTime = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double>(currentTime - lastTime).count();

            // Update every second
            if (elapsed >= 1.0) {
                fps = frame / elapsed;
                std::cout << "FPS: " << fps << "\n";
                frame = 0;
                lastTime = currentTime;
            }
        }

        vkDeviceWaitIdle(m_vulkan_core.get_device());
    }
    
    void Application::clean_up(){
        m_window.clean_up();
        m_vulkan_core.clean_up();
    }
}