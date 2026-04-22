// #############################################################################
// ##                                                                         ##
// ## Logger.hpp                               (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################
#pragma once
#include <string>

class Logger {
public:
    enum class Level {
        Info,
        Warning,
        Error
    };

    static void log(const std::string& message, Level level = Level::Info);
};
