// #############################################################################
// ##                                                                         ##
// ## FlightSimClient.cpp                      (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################


#include "FlightSimClient.hpp"
#include "Logger.hpp"
#include <sstream>

// #############################################################################
FlightSimClient::FlightSimClient() : hFlightSim(nullptr), connected(false) {}

// #############################################################################
FlightSimClient::~FlightSimClient() {
    disconnect();
}

// #############################################################################
bool FlightSimClient::connect(const std::string& appName) {
    if (FAILED(SimConnect_Open(&hFlightSim, appName.c_str(), nullptr, 0, 0, 0))) {
        Logger::log("Failed to connect to MSFS2024.", Logger::Level::Error);
        connected = false;
        return false;
    }
    Logger::log("Connected to MSFS2024 via SimConnect!");
    connected = true;
    return true;
}

// #############################################################################
void FlightSimClient::disconnect() {
    if (hFlightSim) {
        SimConnect_Close(hFlightSim);
        hFlightSim = nullptr;
        connected = false;
    }
}

// #############################################################################
bool FlightSimClient::isConnected() const {
    return connected;
}

// #############################################################################
bool FlightSimClient::mapEvent(DWORD eventId, const char* simEventName) {
    if (!connected) return false;
    HRESULT result = SimConnect_MapClientEventToSimEvent(hFlightSim, eventId, simEventName);
    if (FAILED(result)) {
        std::ostringstream oss;
        oss << "Failed to map event " << simEventName << ". HRESULT: 0x" << std::hex << result;
        Logger::log(oss.str(), Logger::Level::Error);
        return false;
    }
    return true;
}

// #############################################################################
bool FlightSimClient::sendEvent(DWORD eventId, DWORD data) {
    if (!connected) return false;
    HRESULT result = SimConnect_TransmitClientEvent(
        hFlightSim,
        SIMCONNECT_OBJECT_ID_USER,
        eventId,
        data,
        SIMCONNECT_GROUP_PRIORITY_HIGHEST,
        SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY
    );
    if (FAILED(result)) {
        std::ostringstream oss;
        oss << "Event send failed: " << result;
        Logger::log(oss.str(), Logger::Level::Error);
        return false;
    }
    return true;
}
