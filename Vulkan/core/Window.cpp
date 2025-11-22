#include "Window.h"
#include "../utils/Logger.h"

namespace evoke::core {
    void Window::init_window(){
        evoke::utils::Logger::info("Initializing GLFW window!");
        
        glfwInit();
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        
        m_glfw_window = glfwCreateWindow(WIDTH, HEIGHT, NAME, nullptr, nullptr);
        glfwSetWindowUserPointer(m_glfw_window, this);
        
        evoke::utils::Logger::info("Window Width: ", WIDTH);
        evoke::utils::Logger::info("Window Height: ", HEIGHT);
        
        evoke::utils::Logger::info("GLFW window initialization successfull!");
    }
    
    void Window::clean_up(){
        evoke::utils::Logger::info("Cleaning up window!");
        
        glfwDestroyWindow(m_glfw_window);
        glfwTerminate();
        
        evoke::utils::Logger::info("Window cleaned up successfully!");
    }
}
