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
#include "FrequencyControllerCom.hpp"

// #############################################################################
bool MSFSController::isFrequencyStepEvent(EventType type) const {
    return frequencyController && frequencyController->isFrequencyStepEvent(type);
}

// #############################################################################
bool MSFSController::isFlipEvent(EventType type) const {
    return frequencyController && frequencyController->isFlipEvent(type);
}

// #############################################################################
bool MSFSController::isFrequencyRequestEvent(EventType type) const {
    return frequencyController && frequencyController->isFrequencyRequestEvent(type);
}

// #############################################################################
std::string MSFSController::activeInstrumentKey() const {
    return frequencyController ? frequencyController->getInstrumentKey() : "RADIO";
}

// #############################################################################
void MSFSController::queueEvent(EventType type) {
    MsfsEvent evt;
    bool needsInstrumentUpdate = false;
    std::string instrumentKey;
    if (isFrequencyStepEvent(type)) {
        if (!frequencyController) return;
        instrumentKey = activeInstrumentKey();
        if (frequencyController->shouldRequestUpdate()) {
            needsInstrumentUpdate = true;
        }
        evt = frequencyController->createFrequencyEvent(type);
        evt.instrumentKey = instrumentKey;
    } else if (isFlipEvent(type)) {
        if (!frequencyController) return;
        evt = frequencyController->createFlipEvent();
        evt.instrumentKey = activeInstrumentKey();
    } else if (isFrequencyRequestEvent(type)) {
        if (!frequencyController) return;
        evt = frequencyController->requestFrequency();
        evt.instrumentKey = activeInstrumentKey();
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
        MsfsEvent updateEvt = frequencyController->requestFrequency();
        updateEvt.instrumentKey = instrumentKey;
        updateEvt.state = MsfsEventState::Ready;
        eventQueue.push(updateEvt);
        Logger::log("[QUEUE] Queued pending event and async update request for " + instrumentKey);
    } else {
        if (!evt.instrumentKey.empty()) {
            std::ostringstream oss;
            oss.precision(3);
            oss << std::fixed << "[QUEUE] Queuing event: " << evt.simEventName << ", data=" << (evt.data / 1e6) << " MHz (" << evt.data << " Hz)";
            Logger::log(oss.str());
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
                if (frequencyController && isFrequencyStepEvent(evt.type)) {
                    // Rebuild the event using controller logic (config-driven and radio-agnostic).
                    MsfsEvent appliedEvt = frequencyController->createFrequencyEvent(evt.type);
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
    frequencyController = new FrequencyControllerCom();
    frequencyController->setBridge(&bridge);
}

// #############################################################################
void MSFSController::dispatchEvent(const MsfsEvent& evt) {
    if (isFrequencyRequestEvent(evt.type)) {
        // Async: launch frequency fetch in a separate thread
        std::string instrumentKey = evt.instrumentKey;
        if (instrumentKey.empty()) {
            instrumentKey = activeInstrumentKey();
        }
        Logger::log("[ASYNC] Starting async cockpit fetch for " + instrumentKey);
        std::thread([this, instrumentKey, evt]() {
            bool success = false;
            if (frequencyController) {
                unsigned int freq = frequencyController->fetchFreqNonBlocking();
                success = (freq != 0);
            }
            this->markInstrumentUpdateComplete(instrumentKey, success);
        }).detach();
        return;
    }
    if (!evt.instrumentKey.empty()) {
        std::ostringstream oss;
        oss.precision(3);
        oss << std::fixed << "[DISPATCH] Mapping event: " << evt.simEventName << " (ID: 0x"
            << std::hex << evt.eventId << ") with data: " << (evt.data / 1e6) << " MHz (" << evt.data << " Hz)";
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
            std::ostringstream oss;
            oss.precision(3);
            oss << std::fixed << "[DISPATCH] " << evt.name << " dispatched with data: " << (evt.data / 1e6) << " MHz (" << evt.data << " Hz)";
            Logger::log(oss.str());
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
    delete frequencyController;
}

// #############################################################################
void MSFSController::stop() {
    running = false;
}



