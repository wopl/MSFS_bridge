// #############################################################################
// ##                                                                         ##
// ## Logger.cpp                               (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################

#include "Logger.hpp"
#include <iostream>

#include "Logger.hpp"
#include <iostream>

// #############################################################################
void Logger::log(const std::string& message, Level level) {
    switch (level) {
        case Level::Info:
            std::cout << "[INFO] " << message << std::endl;
            break;
        case Level::Warning:
            std::cout << "[WARNING] " << message << std::endl;
            break;
        case Level::Error:
            std::cerr << "[ERROR] " << message << std::endl;
            break;
    }
}
