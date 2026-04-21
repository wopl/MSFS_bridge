#include "FlightSimClient.hpp"

FlightSimClient::FlightSimClient() : hFlightSim(nullptr), connected(false) {}

FlightSimClient::~FlightSimClient() {
    disconnect();
}

bool FlightSimClient::connect(const std::string& appName) {
    if (FAILED(SimConnect_Open(&hFlightSim, appName.c_str(), nullptr, 0, 0, 0))) {
        std::cerr << "Failed to connect to MSFS2024." << std::endl;
        connected = false;
        return false;
    }
    std::cout << "Connected to MSFS2024 via SimConnect!" << std::endl;
    connected = true;
    return true;
}

void FlightSimClient::disconnect() {
    if (hFlightSim) {
        SimConnect_Close(hFlightSim);
        hFlightSim = nullptr;
        connected = false;
    }
}

bool FlightSimClient::isConnected() const {
    return connected;
}

bool FlightSimClient::mapEvent(DWORD eventId, const char* simEventName) {
    if (!connected) return false;
    HRESULT result = SimConnect_MapClientEventToSimEvent(hFlightSim, eventId, simEventName);
    if (FAILED(result)) {
        std::cerr << "Failed to map event " << simEventName << ". HRESULT: 0x" << std::hex << result << std::endl;
        return false;
    }
    return true;
}

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
        std::cerr << "Event send failed: " << result << std::endl;
        return false;
    }
    return true;
}
