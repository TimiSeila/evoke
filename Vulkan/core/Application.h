#pragma once
#include "../renderer/DeviceManager.h"
#include "Window.h"

namespace vbs_engine::core {
class Application {
public:
    void run();
    
private:
    vbs_engine::core::Window m_window;
    vbs_engine::renderer::DeviceManager m_device_manager;
    
    void initApp();
    void main_loop();
    void cleanup();
};
}
