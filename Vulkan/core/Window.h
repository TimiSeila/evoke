#pragma once
#include <GLFW/glfw3.h>
#include <cstdint>

namespace evoke::core {
    class Window{
    public:
        const uint32_t WIDTH = 800;
        const uint32_t HEIGHT = 600;
        const char* NAME = "Evoke";
        
        void init_window();
        void clean_up();
        
        GLFWwindow* get_glfw_window() { return m_glfw_window; }
        
    private:
        GLFWwindow* m_glfw_window;
    };
}
