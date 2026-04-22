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
    MsfEvent createFrequencyEvent(EventType type);
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
};
