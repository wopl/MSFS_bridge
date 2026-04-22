// #############################################################################
// ##                                                                         ##
// ## FrequencyController.cpp                  (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################
#include "FrequencyController.hpp"
#include "EventMap.hpp"
#include "Logger.hpp"
#include <sstream>

// #############################################################################
FrequencyController::FrequencyController()
    : com1_freq(Config::COM1_FREQ_MIN) {
    Logger::log("[FREQ] Initialized com1_freq to " + std::to_string(com1_freq));
}

// #############################################################################
unsigned int FrequencyController::increaseFine() {
    return adjustFine(1);
}
// #############################################################################
unsigned int FrequencyController::decreaseFine() {
    return adjustFine(-1);
}
// #############################################################################
unsigned int FrequencyController::increaseCoarse() {
    return adjustCoarse(1);
}
// #############################################################################
unsigned int FrequencyController::decreaseCoarse() {
    return adjustCoarse(-1);
}

// #############################################################################
unsigned int FrequencyController::adjustFine(int direction) {
    int next_freq = static_cast<int>(com1_freq) + direction * static_cast<int>(Config::COM1_FREQ_FINE_STEP);
    if (next_freq > static_cast<int>(Config::COM1_FREQ_MAX)) {
        Logger::log("[FREQ] Fine step exceeded max, wrapping to min");
        next_freq = Config::COM1_FREQ_MIN;
    } else if (next_freq < static_cast<int>(Config::COM1_FREQ_MIN)) {
        Logger::log("[FREQ] Fine step below min, wrapping to max");
        next_freq = Config::COM1_FREQ_MAX;
    }
    Logger::log("[FREQ] FINE " + std::string(direction > 0 ? "UP" : "DOWN") + ": " + std::to_string(com1_freq) + " -> " + std::to_string(next_freq));
    com1_freq = static_cast<unsigned int>(next_freq);
    return com1_freq;
}

// #############################################################################
unsigned int FrequencyController::adjustCoarse(int direction) {
    int next_freq = static_cast<int>(com1_freq) + direction * static_cast<int>(Config::COM1_FREQ_COARSE_STEP);
    if (next_freq > static_cast<int>(Config::COM1_FREQ_MAX)) {
        Logger::log("[FREQ] Coarse step exceeded max, wrapping to min");
        next_freq = Config::COM1_FREQ_MIN;
    } else if (next_freq < static_cast<int>(Config::COM1_FREQ_MIN)) {
        Logger::log("[FREQ] Coarse step below min, wrapping to max");
        next_freq = Config::COM1_FREQ_MAX;
    }
    Logger::log("[FREQ] COARSE " + std::string(direction > 0 ? "UP" : "DOWN") + ": " + std::to_string(com1_freq) + " -> " + std::to_string(next_freq));
    com1_freq = static_cast<unsigned int>(next_freq);
    return com1_freq;
}

// #############################################################################
unsigned int FrequencyController::getCurrentFreq() const {
    return com1_freq;
}
