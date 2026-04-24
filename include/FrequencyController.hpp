#ifndef FLIGHTSIMBRIDGE_FWD_DECL
#define FLIGHTSIMBRIDGE_FWD_DECL
class FlightSimBridge;
#endif
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

class FrequencyController {
public:
    MsfsEvent createFrequencyEvent(EventType type);
    MsfsEvent requestCom1Frequency(); // New: structured frequency request
    bool shouldRequestUpdate() const; // Async: check if update needed
    unsigned int fetchCom1FreqNonBlocking(); // Async: fetch and update frequency (call from thread)
    FrequencyController();
    unsigned int increaseFine();
    unsigned int decreaseFine();
    unsigned int increaseCoarse();
    unsigned int decreaseCoarse();
    unsigned int getCurrentFreq() const;
    void setBridge(FlightSimBridge* bridgePtr);
private:
    void refreshFreqFromCockpitIfNeeded();
    unsigned int adjustFine(int direction);
    unsigned int adjustCoarse(int direction);
    unsigned int com1_freq;
    std::chrono::steady_clock::time_point lastFreqUpdate;
    FlightSimBridge* bridge = nullptr;
    bool firstFreqEvent = true;
    mutable std::recursive_mutex freqMutex; // Protects com1_freq and related state, now recursive to avoid deadlock
};
