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
#include <iomanip>
#include "FrequencyController.hpp"
#include "FrequencyControllerCom.hpp"
#include "FrequencyControllerNav.hpp"

namespace {

unsigned int fromBcd(unsigned int bcd) {
    unsigned int value = 0;
    unsigned int multiplier = 1;
    while (bcd > 0) {
        value += (bcd & 0xF) * multiplier;
        multiplier *= 10;
        bcd >>= 4;
    }
    return value;
}

std::string formatEventDataForLog(const MsfsEvent& evt) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3);
    if (evt.simEventName == "NAV1_STBY_SET") {
        const unsigned int navFreqTimes100 = fromBcd(evt.data);
        oss << "data=" << (static_cast<double>(navFreqTimes100) / 100.0)
            << " MHz (BCD 0x" << std::hex << std::uppercase << evt.data << std::dec << ")";
        return oss.str();
    }
    oss << "data=" << (evt.data / 1e6) << " MHz (" << evt.data << " Hz)";
    return oss.str();
}

}

// #############################################################################
FrequencyController* MSFSController::controllerForEvent(EventType type) const {
    switch (type) {
        case EventType::NAV1_FREQ_FINE_UP:
        case EventType::NAV1_FREQ_FINE_DOWN:
        case EventType::NAV1_FREQ_COARSE_UP:
        case EventType::NAV1_FREQ_COARSE_DOWN:
        case EventType::NAV1_STBY_FLIP:
        case EventType::REQUEST_NAV1_FREQ:
            return navFrequencyController;
        case EventType::COM1_FREQ_FINE_UP:
        case EventType::COM1_FREQ_FINE_DOWN:
        case EventType::COM1_FREQ_COARSE_UP:
        case EventType::COM1_FREQ_COARSE_DOWN:
        case EventType::COM1_STBY_FLIP:
        case EventType::REQUEST_COM1_FREQ:
            return comFrequencyController;
        default:
            return nullptr;
    }
}

// #############################################################################
FrequencyController* MSFSController::controllerForInstrumentKey(const std::string& instrumentKey) const {
    if (instrumentKey == "COM1") return comFrequencyController;
    if (instrumentKey == "NAV1") return navFrequencyController;
    return nullptr;
}

// #############################################################################
bool MSFSController::isFrequencyStepEvent(EventType type) const {
    FrequencyController* controller = controllerForEvent(type);
    return controller && controller->isFrequencyStepEvent(type);
}

// #############################################################################
bool MSFSController::isFlipEvent(EventType type) const {
    FrequencyController* controller = controllerForEvent(type);
    return controller && controller->isFlipEvent(type);
}

// #############################################################################
bool MSFSController::isFrequencyRequestEvent(EventType type) const {
    FrequencyController* controller = controllerForEvent(type);
    return controller && controller->isFrequencyRequestEvent(type);
}

// #############################################################################
std::string MSFSController::activeInstrumentKey(EventType type) const {
    FrequencyController* controller = controllerForEvent(type);
    return controller ? controller->getInstrumentKey() : "RADIO";
}

// #############################################################################
void MSFSController::queueEvent(EventType type) {
    MsfsEvent evt;
    bool needsInstrumentUpdate = false;
    std::string instrumentKey;
    FrequencyController* controller = controllerForEvent(type);
    if (isFrequencyStepEvent(type)) {
        if (!controller) return;
        instrumentKey = activeInstrumentKey(type);
        if (controller->shouldRequestUpdate()) {
            needsInstrumentUpdate = true;
        }
        evt = controller->createFrequencyEvent(type);
        evt.instrumentKey = instrumentKey;
    } else if (isFlipEvent(type)) {
        if (!controller) return;
        evt = controller->createFlipEvent();
        evt.instrumentKey = activeInstrumentKey(type);
    } else if (isFrequencyRequestEvent(type)) {
        if (!controller) return;
        evt = controller->requestFrequency();
        evt.instrumentKey = activeInstrumentKey(type);
    } else {
        switch (type) {
        case EventType::BRAKE_SET: {
            evt.type = type;
            evt.name = "Parking Brake";
            evt.data = 1;
            for (const auto& entry : eventRegistry) {
                if (entry.type == type) {
                    evt.simEventName = entry.msfsEventName;
                    break;
                }
            }
            auto idIt = msfsEventNameToId.find(evt.simEventName);
            evt.eventId = (idIt != msfsEventNameToId.end()) ? idIt->second : Config::EVENT_PARK_BRAKES_ID;
            if (evt.simEventName.empty()) evt.simEventName = "PARKING_BRAKES";
            break;
        }
        default:
            Logger::log("[QUEUE] Unknown or unhandled EventType");
            return;
        }
    }
    if (needsInstrumentUpdate) {
        evt.state = MsfsEventState::PendingInstrumentUpdate;
        evt.instrumentKey = instrumentKey;
        evt.requestTime = std::chrono::steady_clock::now();
        std::lock_guard<std::mutex> lock(queueMutex);
        pendingEvents.push_back(evt);
        // Queue a generic async update request via controller config.
        MsfsEvent updateEvt = controller->requestFrequency();
        updateEvt.instrumentKey = instrumentKey;
        updateEvt.state = MsfsEventState::Ready;
        eventQueue.push(updateEvt);
        Logger::log("[QUEUE] Queued pending event and async update request for " + instrumentKey);
    } else {
        if (!evt.instrumentKey.empty()) {
            Logger::log("[QUEUE] Queuing event: " + evt.simEventName + ", " + formatEventDataForLog(evt));
        } else {
            Logger::log("[QUEUE] Queuing event: " + evt.simEventName + ", data=" + std::to_string(evt.data));
        }
        queueEvent(evt);
    }
}

// #############################################################################
void MSFSController::queueEvent(const MsfsEvent& evt) {
    std::lock_guard<std::mutex> lock(queueMutex);
    eventQueue.push(evt);
}

// #############################################################################
// Called when an instrument update completes (success or timeout)
void MSFSController::markInstrumentUpdateComplete(const std::string& instrumentKey, bool success) {
    std::lock_guard<std::mutex> lock(queueMutex);
    for (auto& evt : pendingEvents) {
        if (evt.instrumentKey == instrumentKey && evt.state == MsfsEventState::PendingInstrumentUpdate) {
            evt.state = success ? MsfsEventState::Ready : MsfsEventState::FailedTimeout;
            if (success) {
                // After cockpit fetch, apply the original frequency event to the updated state
                FrequencyController* controller = controllerForInstrumentKey(instrumentKey);
                if (controller && controller->isFrequencyStepEvent(evt.type)) {
                    // Rebuild the event using controller logic (config-driven and radio-agnostic).
                    MsfsEvent appliedEvt = controller->createFrequencyEvent(evt.type);
                    evt.name = appliedEvt.name;
                    evt.data = appliedEvt.data;
                    evt.simEventName = appliedEvt.simEventName;
                    evt.eventId = appliedEvt.eventId;
                    std::ostringstream oss;
                    oss << "[ASYNC-DEBUG] Applied pending event after cockpit fetch: type=" << static_cast<int>(evt.type)
                        << ", data=" << evt.data;
                    Logger::log(oss.str());
                }
                eventQueue.push(evt);
                Logger::log("[ASYNC] Marked event ready and queued for " + instrumentKey);
            } else {
                Logger::log("[ASYNC] Event failed due to timeout for " + instrumentKey);
            }
        }
    }
    // Remove processed events
    pendingEvents.erase(std::remove_if(pendingEvents.begin(), pendingEvents.end(),
        [](const MsfsEvent& e) { return e.state != MsfsEventState::PendingInstrumentUpdate; }), pendingEvents.end());
}

// #############################################################################
// Check for pending events that have timed out
void MSFSController::checkPendingEventTimeouts() {
    std::lock_guard<std::mutex> lock(queueMutex);
    auto now = std::chrono::steady_clock::now();
    for (auto& evt : pendingEvents) {
        if (evt.state == MsfsEventState::PendingInstrumentUpdate &&
            std::chrono::duration_cast<std::chrono::milliseconds>(now - evt.requestTime).count() > 1000) {
            evt.state = MsfsEventState::FailedTimeout;
            Logger::log("[ASYNC] Timeout for pending event: " + evt.instrumentKey);
        }
    }
    // Remove processed events
    pendingEvents.erase(std::remove_if(pendingEvents.begin(), pendingEvents.end(),
        [](const MsfsEvent& e) { return e.state != MsfsEventState::PendingInstrumentUpdate; }), pendingEvents.end());
}

// #############################################################################
MSFSController::MSFSController() : bridge() {
    comFrequencyController = new FrequencyControllerCom();
    navFrequencyController = new FrequencyControllerNav();
    comFrequencyController->setBridge(&bridge);
    navFrequencyController->setBridge(&bridge);
}

// #############################################################################
void MSFSController::dispatchEvent(const MsfsEvent& evt) {
    if (isFrequencyRequestEvent(evt.type)) {
        // Async: launch frequency fetch in a separate thread
        std::string instrumentKey = evt.instrumentKey;
        FrequencyController* controller = controllerForEvent(evt.type);
        if (instrumentKey.empty()) {
            instrumentKey = activeInstrumentKey(evt.type);
        }
        Logger::log("[ASYNC] Starting async cockpit fetch for " + instrumentKey);
        std::thread([this, controller, instrumentKey, evt]() {
            bool success = false;
            if (controller) {
                unsigned int freq = controller->fetchFreqNonBlocking();
                success = (freq != 0);
            }
            this->markInstrumentUpdateComplete(instrumentKey, success);
        }).detach();
        return;
    }
    if (!evt.instrumentKey.empty()) {
        std::ostringstream oss;
        oss << "[DISPATCH] Mapping event: " << evt.simEventName << " (ID: 0x"
            << std::hex << evt.eventId << std::dec << ") with " << formatEventDataForLog(evt);
        Logger::log(oss.str());
    } else {
        Logger::log("[DISPATCH] Mapping event: " + evt.simEventName + " (ID: 0x" +
            [&](){ std::ostringstream oss; oss << std::hex << evt.eventId; return oss.str(); }() + ") with data: " + std::to_string(evt.data));
    }
    bool mapped = bridge.mapEvent(evt.eventId, evt.simEventName.c_str());
    if (!mapped) {
        Logger::log("[DISPATCH] mapEvent failed for " + evt.simEventName, Logger::Level::Warning);
    }
    bool sent = bridge.sendEvent(evt.eventId, evt.data);
    if (sent) {
        if (!evt.instrumentKey.empty()) {
            Logger::log("[DISPATCH] " + evt.name + " dispatched with " + formatEventDataForLog(evt));
        } else {
            Logger::log("[DISPATCH] " + evt.name + " dispatched with data: " + std::to_string(evt.data));
        }
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
        checkPendingEventTimeouts();
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            size_t queueSize = eventQueue.size();
            for (size_t i = 0; i < queueSize; ++i) {
                MsfsEvent evt = eventQueue.front();
                eventQueue.pop();
                if (evt.state == MsfsEventState::Ready) {
                    dispatchEvent(evt);
                } else if (evt.state == MsfsEventState::FailedTimeout) {
                    Logger::log("[ASYNC] Dropping event due to timeout: " + evt.simEventName, Logger::Level::Warning);
                } else {
                    // Not ready, requeue
                    eventQueue.push(evt);
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// #############################################################################
MSFSController::~MSFSController() {
    delete comFrequencyController;
    delete navFrequencyController;
}

// #############################################################################
void MSFSController::stop() {
    running = false;
}



