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
    void run();
    void dispatchEvent(const MsfEvent& evt);
    void queueFreqChange(FreqChangeType type);
    unsigned int com1_freq;
private:
    FlightSimBridge bridge;
    std::queue<FreqChangeRequest> freqQueue;
    std::mutex queueMutex;
};

