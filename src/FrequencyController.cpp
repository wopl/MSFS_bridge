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
            setFreqFromHz(cockpitFreq);
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(3);
            oss << "[FREQ] Read COM1 frequency from cockpit: " << (static_cast<double>(cockpitFreq) / 1e6) << " MHz (" << cockpitFreq << " Hz)";
            Logger::log(oss.str());
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
        unsigned int freq = bridge->readCom1Freq();
        setFreqFromHz(freq);
        evt.data = getCurrentFreqHz();
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
    : coarse_mhz(124), fine_band(0) {
    std::lock_guard<std::recursive_mutex> lock(freqMutex);
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3);
    oss << "[FREQ] Initialized coarse_mhz to " << coarse_mhz << ", fine_band to " << fine_band << " (" << getCurrentFreqHz() << " Hz)";
    Logger::log(oss.str());
}

// #############################################################################
unsigned int FrequencyController::increaseFine() {
    std::lock_guard<std::recursive_mutex> lock(freqMutex);
    return adjustFine(1);
}
unsigned int FrequencyController::decreaseFine() {
    std::lock_guard<std::recursive_mutex> lock(freqMutex);
    return adjustFine(-1);
}
unsigned int FrequencyController::increaseCoarse() {
    std::lock_guard<std::recursive_mutex> lock(freqMutex);
    return adjustCoarse(1);
}
unsigned int FrequencyController::decreaseCoarse() {
    std::lock_guard<std::recursive_mutex> lock(freqMutex);
    return adjustCoarse(-1);
}

// #############################################################################
unsigned int FrequencyController::adjustFine(int direction) {
    // Fine band: 0..39 (0=000, 1=025, ..., 39=975)
    int old_fine = static_cast<int>(fine_band);
    int bands = static_cast<int>(Config::COM1_FREQ_FINE_BANDS);
    int new_fine = (old_fine + direction + bands) % bands;
    unsigned int old_hz = getCurrentFreqHz();
    fine_band = static_cast<unsigned int>(new_fine);
    unsigned int new_hz = getCurrentFreqHz();
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3);
    oss << "[FREQ] FINE " << (direction > 0 ? "UP" : "DOWN") << ": "
        << (static_cast<double>(old_hz) / 1e6) << " MHz (" << old_hz << " Hz) -> "
        << (static_cast<double>(new_hz) / 1e6) << " MHz (" << new_hz << " Hz)";
    Logger::log(oss.str());
    return new_hz;
}

// #############################################################################
unsigned int FrequencyController::adjustCoarse(int direction) {
    int old_coarse = static_cast<int>(coarse_mhz);
    int new_coarse = old_coarse + direction;
    // Clamp to valid MHz range
    if (new_coarse < 118) new_coarse = 136;
    if (new_coarse > 136) new_coarse = 118;
    unsigned int old_hz = getCurrentFreqHz();
    coarse_mhz = static_cast<unsigned int>(new_coarse);
    unsigned int new_hz = getCurrentFreqHz();
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3);
    oss << "[FREQ] COARSE " << (direction > 0 ? "UP" : "DOWN") << ": "
        << (static_cast<double>(old_hz) / 1e6) << " MHz (" << old_hz << " Hz) -> "
        << (static_cast<double>(new_hz) / 1e6) << " MHz (" << new_hz << " Hz)";
    Logger::log(oss.str());
    return new_hz;
}

// #############################################################################
unsigned int FrequencyController::getCurrentFreqHz() const {
    std::lock_guard<std::recursive_mutex> lock(freqMutex);
    return coarse_mhz * 1000000 + fine_band * Config::COM1_FREQ_FINE_BAND_STEP;
}
unsigned int FrequencyController::getCoarseMHz() const {
    std::lock_guard<std::recursive_mutex> lock(freqMutex);
    return coarse_mhz;
}
unsigned int FrequencyController::getFineBand() const {
    std::lock_guard<std::recursive_mutex> lock(freqMutex);
    return fine_band;
}
void FrequencyController::setFreqFromHz(unsigned int freq_hz) {
    std::lock_guard<std::recursive_mutex> lock(freqMutex);
    coarse_mhz = freq_hz / 1000000;
    unsigned int rem = freq_hz % 1000000;
    fine_band = rem / Config::COM1_FREQ_FINE_BAND_STEP;
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
            setFreqFromHz(freq);
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
    return getCurrentFreqHz();
}
