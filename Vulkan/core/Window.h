#pragma once
#include <GLFW/glfw3.h>
#include <cstdint>

namespace vbs_engine::core {
class Window{
public:
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;
    void initWindow();
    GLFWwindow* get_glfw_window();
    void set_glfw_window(GLFWwindow* newWindow);

private:
    GLFWwindow* m_glfwWindow;
};
}
