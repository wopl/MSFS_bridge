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

struct FreqChangeRequest {
    FreqChangeSource source;
};

class MSFSController {
public:
    MSFSController();
    void run();
    void dispatchEvent(const MsfEvent& evt);
    void queueFreqChange(FreqChangeSource src);
    unsigned int com1_freq;
private:
    FlightSimBridge bridge;
    std::queue<FreqChangeRequest> freqQueue;
    std::mutex queueMutex;
};

