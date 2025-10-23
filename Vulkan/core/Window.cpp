#include "Window.h"
#include <iostream>

namespace vbs_engine::core {
void Window::initWindow(){
    std::cout << "Initializing GLFW window!\n";
    glfwInit();
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    
    m_glfwWindow = glfwCreateWindow(WIDTH, HEIGHT, "VBS 6", nullptr, nullptr);
    glfwSetWindowUserPointer(m_glfwWindow, this);
    
    std::cout << "GLFW window initialization successfull!\n";
}

GLFWwindow* Window::get_glfw_window(){
    return m_glfwWindow;
}
}
