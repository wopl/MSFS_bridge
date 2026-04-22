// #############################################################################
// ##                                                                         ##
// ## MSFSController.cpp                       (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################
#include "MSFSController.hpp"
#include "EventTypes.hpp"
#include "Logger.hpp"
#include "Config.hpp"
#include <utility>
#include <sstream>
#include "FrequencyController.hpp"

// #############################################################################

void MSFSController::queueEvent(EventType type) {
    MsfEvent evt;
    switch (type) {
        case EventType::COM1_FREQ_FINE_UP:
        case EventType::COM1_FREQ_FINE_DOWN:
        case EventType::COM1_FREQ_COARSE_UP:
        case EventType::COM1_FREQ_COARSE_DOWN:
            if (!frequencyController) return;
            evt = frequencyController->createFrequencyEvent(type);
            break;
        case EventType::BRAKE_SET:
            evt.type = type;
            evt.name = "Parking Brake";
            evt.eventId = EVENT_PARK_BRAKES;
            evt.data = 1;
            evt.simEventName = "PARKING_BRAKES";
            break;
        // Add more cases for other event types as needed
        default:
            Logger::log("[QUEUE] Unknown or unhandled EventType");
            return;
    }
    Logger::log("[QUEUE] Queuing event: " + evt.simEventName + ", data=" + std::to_string(evt.data));
    queueEvent(evt);
}

void MSFSController::queueEvent(const MsfEvent& evt) {
    std::lock_guard<std::mutex> lock(queueMutex);
    eventQueue.push(evt);
}

// #############################################################################
MSFSController::MSFSController() : bridge() {
    frequencyController = new FrequencyController();
}

// #############################################################################
void MSFSController::dispatchEvent(const MsfEvent& evt) {
    Logger::log("[DISPATCH] Mapping event: " + evt.simEventName + " (ID: 0x" +
        [&](){ std::ostringstream oss; oss << std::hex << evt.eventId; return oss.str(); }() + ") with data: " + std::to_string(evt.data));
    bool mapped = bridge.mapEvent(evt.eventId, evt.simEventName.c_str());
    if (!mapped) {
        Logger::log("[DISPATCH] mapEvent failed for " + evt.simEventName, Logger::Level::Warning);
    }
    bool sent = bridge.sendEvent(evt.eventId, evt.data);
    if (sent) {
        Logger::log("[DISPATCH] " + evt.name + " dispatched with data: " + std::to_string(evt.data));
    } else {
        Logger::log("[DISPATCH] sendEvent failed for " + evt.simEventName, Logger::Level::Error);
    }
}

// #############################################################################
void MSFSController::run() {
    if (!bridge.connect()) return;
    running = true;
    // Example: Set parking brake once at startup
    queueEvent(EventType::BRAKE_SET);
    while (running) {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            while (!eventQueue.empty()) {
                MsfEvent evt = eventQueue.front();
                eventQueue.pop();
                dispatchEvent(evt);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// #############################################################################
MSFSController::~MSFSController() {
    delete frequencyController;
}

// #############################################################################
void MSFSController::stop() {
    running = false;
}



