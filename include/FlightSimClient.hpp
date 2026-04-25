// #############################################################################
// ##                                                                         ##
// ## FlightSimClient.hpp                      (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################
#pragma once
#include <windows.h>
#include <SimConnect.h>
#include <string>
#include <unordered_map>


class FlightSimClient {
public:
    FlightSimClient();
    ~FlightSimClient();
    bool connect(const std::string& appName = "MSFS Bridge");
    void disconnect();
    bool isConnected() const;
    bool mapEvent(DWORD eventId, const char* simEventName);
    bool sendEvent(DWORD eventId, DWORD data);
    // Synchronously read COM1 frequency (Hz)
    unsigned int readCom1Freq();
    // Synchronously read NAV1 frequency (Hz)
    unsigned int readNav1Freq();

private:
    HANDLE hFlightSim;
    bool connected;
    std::unordered_map<DWORD, std::string> mappedClientEvents;
};
