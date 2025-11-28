#pragma once
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <chrono>

namespace evoke::utils {
    class Logger {
    public:
        template <typename... Args>
        static void info(Args&&... args) {
            log("[INFO]", std::forward<Args>(args)...);
        }

        template <typename... Args>
        static void error(Args&&... args) {
            log("[ERROR]", std::forward<Args>(args)...);
        }

    private:
        static std::string current_time() {
            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);

            std::stringstream ss;
            ss << std::put_time(std::localtime(&in_time_t), "%H:%M:%S");
            return ss.str();
        }

        template <typename... Args>
        static void log(const std::string& level, Args&&... args) {
            std::ostringstream oss;
            (oss << ... << args);
            std::cout << current_time() << " " << level << " " << oss.str() << "\n";
        }
    };
}