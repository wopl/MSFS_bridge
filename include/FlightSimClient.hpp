// #############################################################################
// ##                                                                         ##
// ## FlightSimClient.hpp                      (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################
#pragma once
#include <windows.h>
#include <SimConnect.h>
#include <string>


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
