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

void FrequencyController::increaseFine() {
    queueChange(FreqChangeType::FINE_UP);
}
void FrequencyController::decreaseFine() {
    queueChange(FreqChangeType::FINE_DOWN);
}
void FrequencyController::increaseCoarse() {
    queueChange(FreqChangeType::COARSE_UP);
}
void FrequencyController::decreaseCoarse() {
    queueChange(FreqChangeType::COARSE_DOWN);
}

void FrequencyController::adjustFine(int direction, MSFSController& msfsController) {
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
    dispatchFreqEvent(direction > 0 ? "FINE_UP" : "FINE_DOWN", msfsController);
}

void FrequencyController::adjustCoarse(int direction, MSFSController& msfsController) {
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
    dispatchFreqEvent(direction > 0 ? "COARSE_UP" : "COARSE_DOWN", msfsController);
}

void FrequencyController::dispatchFreqEvent(const std::string& typeStr, MSFSController& msfsController) {
    MsfEvent freqEvt{"COM1 Frequency (" + typeStr + ")", 0x00011010, com1_freq, "COM_STBY_RADIO_SET_HZ"};
    Logger::log("[FREQ] Dispatching event: " + freqEvt.simEventName + ", data=" + std::to_string(freqEvt.data));
    msfsController.dispatchEvent(freqEvt);
}

void FrequencyController::queueChange(FreqChangeType type) {
    std::lock_guard<std::mutex> lock(queueMutex);
    freqQueue.push(type);
}
// #############################################################################


// #############################################################################
void FrequencyController::processQueue(MSFSController& msfsController) {
    std::lock_guard<std::mutex> lock(queueMutex);
    while (!freqQueue.empty()) {
        FreqChangeType type = freqQueue.front();
        freqQueue.pop();
        int step = 0;
        std::string typeStr;
        switch (type) {
            case FreqChangeType::FINE_UP:
                typeStr = "FINE_UP";
                step = Config::COM1_FREQ_FINE_STEP;
                break;
            case FreqChangeType::FINE_DOWN:
                typeStr = "FINE_DOWN";
                step = -static_cast<int>(Config::COM1_FREQ_FINE_STEP);
                break;
            case FreqChangeType::COARSE_UP:
                typeStr = "COARSE_UP";
                step = Config::COM1_FREQ_COARSE_STEP;
                break;
            case FreqChangeType::COARSE_DOWN:
                typeStr = "COARSE_DOWN";
                step = -static_cast<int>(Config::COM1_FREQ_COARSE_STEP);
                break;
        }
        Logger::log("[DEBUG] Before update: com1_freq=" + std::to_string(com1_freq));
        int prev_freq = static_cast<int>(com1_freq);
        int next_freq = prev_freq + step;
        if (next_freq > static_cast<int>(Config::COM1_FREQ_MAX)) {
            Logger::log("[FREQ] Wrapping to MIN");
            next_freq = Config::COM1_FREQ_MIN;
        }
        if (next_freq < static_cast<int>(Config::COM1_FREQ_MIN)) {
            Logger::log("[FREQ] Wrapping to MAX");
            next_freq = Config::COM1_FREQ_MAX;
        }
        Logger::log("[FREQ] " + typeStr + ": " + std::to_string(com1_freq) + " -> " + std::to_string(next_freq));
        com1_freq = static_cast<unsigned int>(next_freq);
        Logger::log("[DEBUG] Before dispatch: com1_freq=" + std::to_string(com1_freq));
        MsfEvent freqEvt{"COM1 Frequency (" + typeStr + ")", 0x00011010, com1_freq, "COM_STBY_RADIO_SET_HZ"};
        Logger::log("[FREQ] Dispatching event: " + freqEvt.simEventName + ", data=" + std::to_string(freqEvt.data));
        msfsController.dispatchEvent(freqEvt);
    }
}

// #############################################################################
unsigned int FrequencyController::getCurrentFreq() const {
    return com1_freq;
}
