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


class FrequencyController {
public:
    MsfsEvent createFlipEvent();
    MsfsEvent createFrequencyEvent(EventType type);
    MsfsEvent requestFrequency();
    bool shouldRequestUpdate() const;
    unsigned int fetchFreqNonBlocking();
    FrequencyController();
    void increaseFine();
    void decreaseFine();
    void increaseCoarse();
    void decreaseCoarse();
    unsigned int getCurrentFreqHz() const;
    void syncStateFromCockpitHz(unsigned int freq_hz);
    void setBridge(FlightSimBridge* bridgePtr);
    bool isFrequencyStepEvent(EventType type) const;
    bool isFlipEvent(EventType type) const;
    bool isFrequencyRequestEvent(EventType type) const;
    std::string getInstrumentKey() const;
protected:
    virtual const Config::RadioConfig& getConfig() const = 0;
    virtual unsigned int readStandbyFreqFromBridge(FlightSimBridge& bridge) const = 0;
private:
    void refreshFreqFromCockpitIfNeeded();
    int freq_coarse;
    int freq_fine;
    std::chrono::steady_clock::time_point lastFreqUpdate;
    bool firstFreqEvent = true;
    FlightSimBridge* bridge = nullptr;
    mutable std::recursive_mutex freqMutex;
};

