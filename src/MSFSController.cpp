// #############################################################################
// ##                                                                         ##
// ## MSFSController.cpp                       (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################
#include "MSFSController.hpp"
#include "Logger.hpp"
#include "Config.hpp"
#include <utility>
#include <sstream>


#include "FrequencyController.hpp"

// #############################################################################
void MSFSController::queueFreqChange(FreqChangeType type) {
    if (frequencyController) {
        switch (type) {
            case FreqChangeType::FINE_UP:
                frequencyController->increaseFine();
                break;
            case FreqChangeType::FINE_DOWN:
                frequencyController->decreaseFine();
                break;
            case FreqChangeType::COARSE_UP:
                frequencyController->increaseCoarse();
                break;
            case FreqChangeType::COARSE_DOWN:
                frequencyController->decreaseCoarse();
                break;
        }
    }
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
    MsfEvent brakeEvt{"Parking Brake", EVENT_PARK_BRAKES, 1, "PARKING_BRAKES"};
    dispatchEvent(brakeEvt);
    while (running) {
        if (frequencyController) {
            frequencyController->processQueue(*this);
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


