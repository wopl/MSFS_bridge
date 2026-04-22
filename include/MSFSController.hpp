// #############################################################################
// ##                                                                         ##
// ## MSFSController.hpp                       (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################
#pragma once
#include "FlightSimBridge.hpp"
#include <thread>
#include <chrono>
#include <iostream>

#include "msfs_events.hpp"
#include <queue>
#include <mutex>

struct MsfEvent {
    std::string name;
    unsigned int eventId;
    unsigned int data;
    std::string simEventName;
};

enum class FreqChangeSource {
    UDP,
    TIMER
};

enum class FreqChangeType {
    FINE_UP,
    FINE_DOWN,
    COARSE_UP,
    COARSE_DOWN
};

struct FreqChangeRequest {
    FreqChangeType type;
};

class MSFSController {
public:
    MSFSController();
    ~MSFSController();
    void run();
    void stop();
    void dispatchEvent(const MsfEvent& evt);
    void queueFreqChange(FreqChangeType type);
    // Add more general queue methods for other controls as needed
private:
    FlightSimBridge bridge;
    std::atomic<bool> running{false};
    // Control modules
    class FrequencyController* frequencyController;
};

