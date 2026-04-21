#pragma once
#include <windows.h>
#include <SimConnect.h>
#include <string>
#include <iostream>

class FlightSimClient {
public:
    FlightSimClient();
    ~FlightSimClient();
    bool connect(const std::string& appName = "MSFS Bridge");
    void disconnect();
    bool isConnected() const;
    bool mapEvent(DWORD eventId, const char* simEventName);
    bool sendEvent(DWORD eventId, DWORD data);

private:
    HANDLE hFlightSim;
    bool connected;
};
