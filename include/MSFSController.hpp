#pragma once
#include "FlightSimBridge.hpp"
#include <thread>
#include <chrono>
#include <iostream>
#include "msfs_events.hpp"


struct MsfEvent {
    std::string name;
    unsigned int eventId;
    unsigned int data;
    std::string simEventName;
};

class MSFSController {
public:
    MSFSController();
    void run();
    void dispatchEvent(const MsfEvent& evt);
private:
    FlightSimBridge bridge;
    unsigned int com1_freq;
};
