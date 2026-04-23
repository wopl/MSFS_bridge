// #############################################################################
// ##                                                                         ##
// ## FlightSimBridge.hpp                      (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################
#pragma once
#include "FlightSimClient.hpp"

class FlightSimBridge {
public:
    FlightSimBridge();
    ~FlightSimBridge();
    void run();
    bool mapEvent(unsigned int eventId, const char* simEventName);
    bool sendEvent(unsigned int eventId, unsigned int data);
    bool connect();
    void disconnect();
    bool isConnected() const;
    // Expose synchronous COM1 frequency read
    unsigned int readCom1Freq();

private:
    FlightSimClient flightSimClient;
};
