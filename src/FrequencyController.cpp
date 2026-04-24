// #############################################################################
// ##                                                                         ##
// ## FrequencyController.cpp                  (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################
#include "FrequencyController.hpp"
#include "EventTypes.hpp"
#include "FlightSimBridge.hpp"
#include "Logger.hpp"
#include <chrono>
#include <mutex>
#include <sstream>
#include <iomanip>
// #############################################################################
bool FrequencyController::shouldRequestUpdate() const {
    std::lock_guard<std::recursive_mutex> lock(freqMutex);
    using namespace std::chrono;
    auto now = steady_clock::now();
    return firstFreqEvent || (duration_cast<seconds>(now - lastFreqUpdate).count() > 30);
}

// #############################################################################
void FrequencyController::setBridge(FlightSimBridge* bridgePtr) {
    std::lock_guard<std::recursive_mutex> lock(freqMutex);
    bridge = bridgePtr;
}

// #############################################################################
void FrequencyController::refreshFreqFromCockpitIfNeeded() {
    std::lock_guard<std::recursive_mutex> lock(freqMutex);
    using namespace std::chrono;
    auto now = steady_clock::now();
    bool needRead = firstFreqEvent || (std::chrono::duration_cast<std::chrono::seconds>(now - lastFreqUpdate).count() > 30);
    if (needRead && bridge && bridge->isConnected()) {
        unsigned int cockpitFreq = bridge->readCom1Freq();
        if (cockpitFreq != 0) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(3);
            oss << "[FREQ] Read COM1 frequency from cockpit: " << (static_cast<double>(cockpitFreq) / 1e6) << " MHz (" << cockpitFreq << " Hz)";
            Logger::log(oss.str());
            com1_freq = cockpitFreq;
        } else {
            Logger::log("[FREQ] Failed to read COM1 frequency from cockpit", Logger::Level::Warning);
        }
        lastFreqUpdate = now;
        firstFreqEvent = false;
    }
}
#include <sstream>

// #############################################################################
MsfsEvent FrequencyController::requestCom1Frequency() {
    std::lock_guard<std::recursive_mutex> lock(freqMutex);
    MsfsEvent evt;
    evt.type = EventType::REQUEST_COM1_FREQ;
    evt.name = "Request COM1 Frequency";
    evt.simEventName = "COM1_FREQ_REQUEST";
    auto idIt = msfsEventNameToId.find(evt.simEventName);
    evt.eventId = (idIt != msfsEventNameToId.end()) ? idIt->second : 0;
    evt.data = 0;
    if (bridge && bridge->isConnected()) {
        evt.data = bridge->readCom1Freq();
        com1_freq = evt.data;
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3);
        oss << "[FREQ] Requested COM1 frequency from cockpit: " << (static_cast<double>(evt.data) / 1e6) << " MHz (" << evt.data << " Hz)";
        Logger::log(oss.str());
        lastFreqUpdate = std::chrono::steady_clock::now();
    } else {
        Logger::log("[FREQ] Bridge not connected, cannot request COM1 frequency", Logger::Level::Warning);
    }
    return evt;
}

// #############################################################################
FrequencyController::FrequencyController()
    : com1_freq(Config::COM1_FREQ_MIN) {
    std::lock_guard<std::recursive_mutex> lock(freqMutex);
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3);
    oss << "[FREQ] Initialized com1_freq to " << (static_cast<double>(com1_freq) / 1e6) << " MHz (" << com1_freq << " Hz)";
    Logger::log(oss.str());
}

// #############################################################################
unsigned int FrequencyController::increaseFine() {
    std::lock_guard<std::recursive_mutex> lock(freqMutex);
    return adjustFine(1);
}
// #############################################################################
unsigned int FrequencyController::decreaseFine() {
    std::lock_guard<std::recursive_mutex> lock(freqMutex);
    return adjustFine(-1);
}
// #############################################################################
unsigned int FrequencyController::increaseCoarse() {
    std::lock_guard<std::recursive_mutex> lock(freqMutex);
    return adjustCoarse(1);
}
// #############################################################################
unsigned int FrequencyController::decreaseCoarse() {
    std::lock_guard<std::recursive_mutex> lock(freqMutex);
    return adjustCoarse(-1);
}

// #############################################################################
unsigned int FrequencyController::adjustFine(int direction) {
    int next_freq = static_cast<int>(com1_freq) + direction * static_cast<int>(Config::COM1_FREQ_FINE_STEP);
    if (next_freq > static_cast<int>(Config::COM1_FREQ_MAX)) {
        Logger::log("[FREQ] Fine step exceeded max, wrapping to min");
        next_freq = static_cast<int>(Config::COM1_FREQ_MIN);
    } else if (next_freq < static_cast<int>(Config::COM1_FREQ_MIN)) {
        Logger::log("[FREQ] Fine step below min, wrapping to max");
        next_freq = static_cast<int>(Config::COM1_FREQ_MAX);
    }
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3);
        oss << "[FREQ] FINE " << (direction > 0 ? "UP" : "DOWN") << ": "
            << (static_cast<double>(com1_freq) / 1e6) << " MHz (" << com1_freq << " Hz) -> "
            << (static_cast<double>(next_freq) / 1e6) << " MHz (" << next_freq << " Hz)";
        Logger::log(oss.str());
    }
    com1_freq = static_cast<unsigned int>(next_freq);
    return com1_freq;
}

// #############################################################################
unsigned int FrequencyController::adjustCoarse(int direction) {
    int next_freq = static_cast<int>(com1_freq) + direction * static_cast<int>(Config::COM1_FREQ_COARSE_STEP);
    if (next_freq > static_cast<int>(Config::COM1_FREQ_MAX)) {
        Logger::log("[FREQ] Coarse step exceeded max, wrapping to min");
        next_freq = static_cast<int>(Config::COM1_FREQ_MIN);
    } else if (next_freq < static_cast<int>(Config::COM1_FREQ_MIN)) {
        Logger::log("[FREQ] Coarse step below min, wrapping to max");
        next_freq = static_cast<int>(Config::COM1_FREQ_MAX);
    }
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3);
        oss << "[FREQ] COARSE " << (direction > 0 ? "UP" : "DOWN") << ": "
            << (static_cast<double>(com1_freq) / 1e6) << " MHz (" << com1_freq << " Hz) -> "
            << (static_cast<double>(next_freq) / 1e6) << " MHz (" << next_freq << " Hz)";
        Logger::log(oss.str());
    }
    com1_freq = static_cast<unsigned int>(next_freq);
    return com1_freq;
}

// #############################################################################
unsigned int FrequencyController::getCurrentFreq() const {
    std::lock_guard<std::recursive_mutex> lock(freqMutex);
    return com1_freq;
}

// #############################################################################
MsfsEvent FrequencyController::createFrequencyEvent(EventType type) {
    std::lock_guard<std::recursive_mutex> lock(freqMutex);
    refreshFreqFromCockpitIfNeeded();
    MsfsEvent evt;
    evt.type = type;
    // Lookup event name from eventRegistry
    for (const auto& entry : eventRegistry) {
        if (entry.type == type) {
            evt.simEventName = entry.msfsEventName;
            break;
        }
    }
    switch (type) {
        case EventType::COM1_FREQ_FINE_UP:
            evt.name = "COM1 Frequency (FINE_UP)";
            evt.data = increaseFine();
            break;
        case EventType::COM1_FREQ_FINE_DOWN:
            evt.name = "COM1 Frequency (FINE_DOWN)";
            evt.data = decreaseFine();
            break;
        case EventType::COM1_FREQ_COARSE_UP:
            evt.name = "COM1 Frequency (COARSE_UP)";
            evt.data = increaseCoarse();
            break;
        case EventType::COM1_FREQ_COARSE_DOWN:
            evt.name = "COM1 Frequency (COARSE_DOWN)";
            evt.data = decreaseCoarse();
            break;
        default:
            Logger::log("[FREQ] Unknown frequency EventType");
            break;
    }
    return evt;
}

// #############################################################################
unsigned int FrequencyController::fetchCom1FreqNonBlocking() {
    std::lock_guard<std::recursive_mutex> lock(freqMutex);
    unsigned int freq = 0;
    if (bridge && bridge->isConnected()) {
        freq = bridge->readCom1Freq();
        if (freq != 0) {
            com1_freq = freq;
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(3);
            oss << "[ASYNC] Non-blocking fetch: COM1 frequency updated to " << (static_cast<double>(freq) / 1e6) << " MHz (" << freq << " Hz)";
            Logger::log(oss.str());
            lastFreqUpdate = std::chrono::steady_clock::now();
            firstFreqEvent = false;
        } else {
            Logger::log("[ASYNC] Non-blocking fetch failed to read COM1 frequency", Logger::Level::Warning);
        }
    }
    return freq;
}
