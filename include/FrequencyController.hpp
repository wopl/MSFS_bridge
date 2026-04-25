// #############################################################################
// ##                                                                         ##
// ## FrequencyController.hpp                  (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################
#pragma once
#include <queue>
#include <mutex>
#include "EventTypes.hpp"
#include "Config.hpp"
#include "FlightSimBridge.hpp"
#include <chrono>
#include <string>


// --- Restored FrequencyController for COM1 and NAV1 (fully functional) ---
class FrequencyController {
public:
    // COM1
    MsfsEvent createCom1FlipEvent();
    MsfsEvent createFrequencyEvent(EventType type);
    MsfsEvent requestCom1Frequency();
    bool shouldRequestUpdate() const;
    unsigned int fetchCom1FreqNonBlocking();
    // NAV1
    MsfsEvent createNav1FlipEvent();
    MsfsEvent createNav1FrequencyEvent(EventType type);
    MsfsEvent requestNav1Frequency();
    bool shouldRequestNav1Update() const;
    unsigned int fetchNav1FreqNonBlocking();
    // Construction
    FrequencyController();
    // COM1 helpers
    void increaseFine();
    void decreaseFine();
    void increaseCoarse();
    void decreaseCoarse();
    unsigned int getCurrentFreqHz() const;
    void setFreqFromHz(unsigned int freq_hz);
    // NAV1 helpers
    void increaseNav1Fine();
    void decreaseNav1Fine();
    void increaseNav1Coarse();
    void decreaseNav1Coarse();
    unsigned int getCurrentNav1FreqHz() const;
    void setNav1FreqFromHz(unsigned int freq_hz);
    // Bridge
    void setBridge(FlightSimBridge* bridgePtr);
protected:
    virtual const Config::RadioConfig& getConfig() const = 0;
private:
    void refreshFreqFromCockpitIfNeeded();
    void refreshNav1FreqFromCockpitIfNeeded();
    // COM1 state
    int freq_coarse; // 118..136
    int freq_fine;   // 0..999 (in steps of 5)
    std::chrono::steady_clock::time_point lastFreqUpdate;
    bool firstFreqEvent = true;
    // NAV1 state
    int nav1_freq_coarse; // 108..117
    int nav1_freq_fine;   // 0..999 (in steps of 10)
    std::chrono::steady_clock::time_point lastNav1FreqUpdate;
    bool firstNav1FreqEvent = true;
    // Shared
    FlightSimBridge* bridge = nullptr;
    mutable std::recursive_mutex freqMutex;
};

