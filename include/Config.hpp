// #############################################################################
// ##                                                                         ##
// ## Config.hpp                               (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################
#pragma once
#include <string>

namespace Config {
    // UDP
    constexpr int UDP_PORT = 49000;

    // COM1 Radio Commands
    constexpr const char* CMD_COM1_STBY_FINE_DOWN_833 = "sim/radios/stby_com1_fine_down_833";
    constexpr const char* CMD_COM1_STBY_FINE_UP_833 = "sim/radios/stby_com1_fine_up_833";
    constexpr const char* CMD_COM1_STBY_COARSE_DOWN_833 = "sim/radios/stby_com1_coarse_down_833";
    constexpr const char* CMD_COM1_STBY_COARSE_UP_833 = "sim/radios/stby_com1_coarse_up_833";
    constexpr const char* CMD_COM1_FLIP = "sim/radios/com1_standy_flip";
    constexpr const char* CMD_COM1_FREQ_UP = "sim/radios/com1_freq_up_833";

    // Frequency bounds
    constexpr unsigned int COM1_FREQ_MIN = 118000000;
    constexpr unsigned int COM1_FREQ_MAX = 136990000;
    constexpr unsigned int COM1_FREQ_FINE_STEP = 5000;
    constexpr unsigned int COM1_FREQ_COARSE_STEP = 1000000;


    // Add more configuration constants as needed
}
