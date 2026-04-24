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

// Helper: extract freq_coarse and freq_fine from Hz
void FrequencyController::setFreqFromHz(unsigned int freq_hz) {
    freq_coarse = static_cast<int>(freq_hz / 1000000);
    int remainder = static_cast<int>(freq_hz % 1000000);
    // Round down to nearest 5 kHz step
    freq_fine = (remainder / 1000) / 5 * 5;
}
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
    bool needRead = firstFreqEvent || (duration_cast<seconds>(now - lastFreqUpdate).count() > 30);
    if (needRead && bridge && bridge->isConnected()) {
        unsigned int cockpitFreq = bridge->readCom1Freq();
        if (cockpitFreq != 0) {
            setFreqFromHz(cockpitFreq);
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(3);
            oss << "[FREQ] Read COM1 frequency from cockpit: " << (static_cast<double>(cockpitFreq) / 1e6) << " MHz (" << cockpitFreq << " Hz)";
            oss << ", freq_coarse: " << freq_coarse << ", freq_fine: " << freq_fine;
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
        evt.data = static_cast<unsigned int>((freq_coarse * 1000000) + (freq_fine * 1000));
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
    : freq_coarse(124), freq_fine(0) {
    std::lock_guard<std::recursive_mutex> lock(freqMutex);
        // Initialization log removed
}
// --- Frequency step helpers ---
void FrequencyController::increaseCoarse() {
    freq_coarse++;
    if (freq_coarse > 136) freq_coarse = 118;
}

void FrequencyController::decreaseCoarse() {
    freq_coarse--;
    if (freq_coarse < 118) freq_coarse = 136;
}

void FrequencyController::increaseFine() {
    freq_fine += 5;
    if (freq_fine > 999) freq_fine = 0;
}

void FrequencyController::decreaseFine() {
    freq_fine -= 5;
    if (freq_fine < 0) freq_fine = 995;
}

// #############################################################################
// setFreqFromHz is implemented above using freq_coarse and freq_fine only.

// #############################################################################
MsfsEvent FrequencyController::createFrequencyEvent(EventType type) {
    std::lock_guard<std::recursive_mutex> lock(freqMutex);

    refreshFreqFromCockpitIfNeeded();

    // Apply the adjustment to the internal state
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
            increaseFine();
            break;
        case EventType::COM1_FREQ_FINE_DOWN:
            evt.name = "COM1 Frequency (FINE_DOWN)";
            decreaseFine();
            break;
        case EventType::COM1_FREQ_COARSE_UP:
            evt.name = "COM1 Frequency (COARSE_UP)";
            increaseCoarse();
            break;
        case EventType::COM1_FREQ_COARSE_DOWN:
            evt.name = "COM1 Frequency (COARSE_DOWN)";
            decreaseCoarse();
            break;
        default:
            Logger::log("[FREQ] Unknown frequency EventType");
            break;
    }
    // Prepare data for transmission (Hz)
    evt.data = static_cast<unsigned int>((freq_coarse * 1000000) + (freq_fine * 1000));
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
    return static_cast<unsigned int>((freq_coarse * 1000000) + (freq_fine * 1000));
}
