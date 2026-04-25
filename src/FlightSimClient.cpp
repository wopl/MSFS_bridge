// #############################################################################
// ##                                                                         ##
// ## FlightSimClient.cpp                      (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################


#include "FlightSimClient.hpp"
#include "Logger.hpp"
#include <sstream>

#pragma pack(push, 1)
struct Com1FreqStruct {
    double com1_freq;
};
#pragma pack(pop)

// SimConnect data definition IDs and request IDs for COM1 and NAV1 frequency
enum : DWORD {
    DATA_DEFINITION_COM1_FREQ = 1,
    DATA_REQUEST_COM1_FREQ = 1,
    DATA_DEFINITION_NAV1_FREQ = 2,
    DATA_REQUEST_NAV1_FREQ = 2
};

// #############################################################################
unsigned int FlightSimClient::readCom1Freq() {
    if (!connected || !hFlightSim) return 0;
    HRESULT hr;
    hr = SimConnect_AddToDataDefinition(hFlightSim, DATA_DEFINITION_COM1_FREQ, "COM STANDBY FREQUENCY:1", "MHz");
    if (FAILED(hr)) {
        Logger::log("[SimConnect] Failed to add data definition for COM1 freq", Logger::Level::Error);
        return 0;
    }
    hr = SimConnect_RequestDataOnSimObject(hFlightSim, DATA_REQUEST_COM1_FREQ, DATA_DEFINITION_COM1_FREQ, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_ONCE);
    if (FAILED(hr)) {
        Logger::log("[SimConnect] Failed to request COM1 freq", Logger::Level::Error);
        return 0;
    }
    Com1FreqStruct freqData = {0};
    bool gotData = false;
    DWORD startTick = GetTickCount();
    while (!gotData && (GetTickCount() - startTick < 3000)) {
        SIMCONNECT_RECV* pData = nullptr;
        DWORD cbData = 0;
        hr = SimConnect_GetNextDispatch(hFlightSim, &pData, &cbData);
        if (SUCCEEDED(hr) && pData) {
            if (pData->dwID == SIMCONNECT_RECV_ID_SIMOBJECT_DATA) {
                auto* pObj = reinterpret_cast<SIMCONNECT_RECV_SIMOBJECT_DATA*>(pData);
                if (pObj->dwRequestID == DATA_REQUEST_COM1_FREQ) {
                    freqData = *reinterpret_cast<Com1FreqStruct*>(&pObj->dwData);
                    gotData = true;
                }
            }
        }
        Sleep(10);
    }
    if (!gotData) {
        Logger::log("[SimConnect] Timeout waiting for COM1 freq", Logger::Level::Warning);
        return 0;
    }
    unsigned int freq = static_cast<unsigned int>(freqData.com1_freq * 1000.0 + 0.5) * 1000;
    std::ostringstream oss;
    oss.precision(3);
    oss << std::fixed << "[SimConnect] Read COM1 freq: " << (freqData.com1_freq) << " MHz (" << freq << " Hz)";
    Logger::log(oss.str());
    return freq;
}

// #############################################################################
unsigned int FlightSimClient::readNav1Freq() {
    if (!connected || !hFlightSim) return 0;
    HRESULT hr;
    hr = SimConnect_AddToDataDefinition(hFlightSim, DATA_DEFINITION_NAV1_FREQ, "NAV STANDBY FREQUENCY:1", "MHz");
    if (FAILED(hr)) {
        Logger::log("[SimConnect] Failed to add data definition for NAV1 freq", Logger::Level::Error);
        return 0;
    }
    hr = SimConnect_RequestDataOnSimObject(hFlightSim, DATA_REQUEST_NAV1_FREQ, DATA_DEFINITION_NAV1_FREQ, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_ONCE);
    if (FAILED(hr)) {
        Logger::log("[SimConnect] Failed to request NAV1 freq", Logger::Level::Error);
        return 0;
    }
    Com1FreqStruct freqData = {0};
    bool gotData = false;
    DWORD startTick = GetTickCount();
    while (!gotData && (GetTickCount() - startTick < 3000)) {
        SIMCONNECT_RECV* pData = nullptr;
        DWORD cbData = 0;
        hr = SimConnect_GetNextDispatch(hFlightSim, &pData, &cbData);
        if (SUCCEEDED(hr) && pData) {
            if (pData->dwID == SIMCONNECT_RECV_ID_SIMOBJECT_DATA) {
                auto* pObj = reinterpret_cast<SIMCONNECT_RECV_SIMOBJECT_DATA*>(pData);
                if (pObj->dwRequestID == DATA_REQUEST_NAV1_FREQ) {
                    freqData = *reinterpret_cast<Com1FreqStruct*>(&pObj->dwData);
                    gotData = true;
                }
            }
        }
        Sleep(10);
    }
    if (!gotData) {
        Logger::log("[SimConnect] Timeout waiting for NAV1 freq", Logger::Level::Warning);
        return 0;
    }
    unsigned int freq = static_cast<unsigned int>(freqData.com1_freq * 1000.0 + 0.5) * 1000;
    std::ostringstream oss;
    oss.precision(3);
    oss << std::fixed << "[SimConnect] Read NAV1 freq: " << (freqData.com1_freq) << " MHz (" << freq << " Hz)";
    Logger::log(oss.str());
    return freq;
}

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
        mappedClientEvents.clear();
    }
}

// #############################################################################
bool FlightSimClient::isConnected() const {
    return connected;
}

// #############################################################################
bool FlightSimClient::mapEvent(DWORD eventId, const char* simEventName) {
    if (!connected) return false;
    const std::string simEvent = (simEventName != nullptr) ? simEventName : "";

    auto existing = mappedClientEvents.find(eventId);
    if (existing != mappedClientEvents.end()) {
        if (existing->second == simEvent) {
            return true;
        }
        std::ostringstream oss;
        oss << "Client event ID 0x" << std::hex << eventId
            << " already mapped to '" << existing->second
            << "', cannot remap to '" << simEvent << "'";
        Logger::log(oss.str(), Logger::Level::Error);
        return false;
    }

    HRESULT result = SimConnect_MapClientEventToSimEvent(hFlightSim, eventId, simEventName);
    if (FAILED(result)) {
        std::ostringstream oss;
        oss << "Failed to map event " << simEventName << ". HRESULT: 0x" << std::hex << result;
        Logger::log(oss.str(), Logger::Level::Error);
        return false;
    }

    mappedClientEvents[eventId] = simEvent;
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
