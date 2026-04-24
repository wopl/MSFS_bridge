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
    MsfsEvent requestCom1Frequency();
    bool shouldRequestUpdate() const;
    unsigned int fetchCom1FreqNonBlocking();
    FrequencyController();
    unsigned int increaseFine();
    unsigned int decreaseFine();
    unsigned int increaseCoarse();
    unsigned int decreaseCoarse();
    unsigned int getCurrentFreqHz() const; // Returns Hz
    unsigned int getCoarseMHz() const;     // Returns MHz
    unsigned int getFineBand() const;      // Returns 0,1,2,3
    void setBridge(FlightSimBridge* bridgePtr);
    void setFreqFromHz(unsigned int freq_hz); // Set state from Hz
private:
    void refreshFreqFromCockpitIfNeeded();
    unsigned int adjustFine(int direction);
    unsigned int adjustCoarse(int direction);
    unsigned int coarse_mhz; // e.g. 124
    unsigned int fine_band;  // 0,1,2,3 (0=000, 1=025, 2=050, 3=075)
    std::chrono::steady_clock::time_point lastFreqUpdate;
    FlightSimBridge* bridge = nullptr;
    bool firstFreqEvent = true;
    mutable std::recursive_mutex freqMutex;
};
