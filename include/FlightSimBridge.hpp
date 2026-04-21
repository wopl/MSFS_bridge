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

private:
    FlightSimClient flightSimClient;
};
