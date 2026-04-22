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
void FrequencyController::queueChange(FreqChangeType type) {
    std::lock_guard<std::mutex> lock(queueMutex);
    freqQueue.push(type);
}

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
        int prev_freq = static_cast<int>(com1_freq);
        int next_freq = prev_freq + step;
        Logger::log("[FREQ] Processing " + typeStr + ": prev_freq=" + std::to_string(prev_freq) + ", step=" + std::to_string(step));
        if (next_freq > static_cast<int>(Config::COM1_FREQ_MAX)) {
            Logger::log("[FREQ] Wrapping to MIN");
            next_freq = Config::COM1_FREQ_MIN;
        }
        if (next_freq < static_cast<int>(Config::COM1_FREQ_MIN)) {
            Logger::log("[FREQ] Wrapping to MAX");
            next_freq = Config::COM1_FREQ_MAX;
        }
        com1_freq = static_cast<unsigned int>(next_freq);
        Logger::log("[FREQ] Updated com1_freq=" + std::to_string(com1_freq));
        // Dispatch event via MSFSController (SimConnect expects Hz)
        MsfEvent freqEvt{"COM1 Frequency (" + typeStr + ")", 0x00011010, com1_freq, "COM_STBY_RADIO_SET_HZ"};
        std::ostringstream oss;
        oss << "[FREQ] Dispatching event: " << freqEvt.simEventName << ", eventId=0x" << std::hex << freqEvt.eventId << ", data=" << std::dec << freqEvt.data;
        Logger::log(oss.str());
        msfsController.dispatchEvent(freqEvt);
    }
}

// #############################################################################
unsigned int FrequencyController::getCurrentFreq() const {
    return com1_freq;
}
