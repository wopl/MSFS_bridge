// #############################################################################
// ##                                                                         ##
// ## MSFSController.hpp                       (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################
#pragma once
#include "FlightSimBridge.hpp"
#include "EventTypes.hpp"
#include <thread>
#include <chrono>
#include <iostream>
#include <queue>
#include <mutex>

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
    void dispatchEvent(const MsfsEvent& evt);
    void queueEvent(EventType type);
    void queueGenericEvent(const std::string& eventName, unsigned int eventId, unsigned int data, const std::string& simEventName);
    void queueEvent(const MsfsEvent& evt);
    // Async event management
    void markInstrumentUpdateComplete(const std::string& instrumentKey, bool success);
    void checkPendingEventTimeouts();
private:
    bool isFrequencyStepEvent(EventType type) const;
    bool isFlipEvent(EventType type) const;
    bool isFrequencyRequestEvent(EventType type) const;
    std::string activeInstrumentKey() const;
    FlightSimBridge bridge;
    std::atomic<bool> running{false};
    // Control modules
    class FrequencyController* frequencyController;
    std::queue<MsfsEvent> eventQueue;
    std::vector<MsfsEvent> pendingEvents; // Events waiting for instrument update
    std::mutex queueMutex;
};

