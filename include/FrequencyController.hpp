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

class FrequencyController {
public:
        /**
         * Create an event for COM1 standby flip (active/standby swap).
         */
        MsfsEvent createCom1FlipEvent();
    /**
     * Create a frequency event for the given type (fine/coarse up/down).
     */
    MsfsEvent createFrequencyEvent(EventType type);
    /**
     * Request the current COM1 frequency from the cockpit (syncs state).
     */
    MsfsEvent requestCom1Frequency();
    /**
     * Check if a frequency update is needed (first event or >30s).
     */
    bool shouldRequestUpdate() const;
    /**
     * Fetch and update frequency asynchronously (call from thread).
     */
    unsigned int fetchCom1FreqNonBlocking();
    /**
     * Construct with default frequency (124.000 MHz).
     */
    FrequencyController();
    // Increase/decrease helpers for freq_fine and freq_coarse (void return type)
    void increaseFine();
    void decreaseFine();
    void increaseCoarse();
    void decreaseCoarse();

    /**
     * Get the current frequency in Hz (thread-safe).
     */
    unsigned int getCurrentFreqHz() const;
    // No frequency calculation methods needed; freq_coarse and freq_fine are the only state.
        /**
         * Set the bridge pointer for cockpit communication.
         */
        void setBridge(FlightSimBridge* bridgePtr);
        /**
         * Set frequency state from Hz value (splits into coarse/fine).
         */
        void setFreqFromHz(unsigned int freq_hz);
private:
    void refreshFreqFromCockpitIfNeeded();
    // Frequency state (encapsulated):
    int freq_coarse; // 118..136
    int freq_fine;   // 0..999 (in steps of 5)
    std::chrono::steady_clock::time_point lastFreqUpdate;
    FlightSimBridge* bridge = nullptr;
    bool firstFreqEvent = true;
    mutable std::recursive_mutex freqMutex;

    // --- Frequency math helpers ---
    // (No obsolete frequency calculation helpers remain)
};
