#include "Application.h"
#include <iostream>

namespace vbs_engine::core {
void Application::run() {
    initApp();
    main_loop();
    //cleanup();
}

void Application::initApp() {
    std::cout << "Initializing application!\n";
    m_window.initWindow();
    m_device_manager.initDeviceManager();
    std::cout << "Application initialized successfully!\n";
}

void Application::main_loop(){    
    while (!glfwWindowShouldClose(m_window.get_glfw_window())) {
        glfwPollEvents();
    }
}

void Application::cleanup(){
    
}
}
