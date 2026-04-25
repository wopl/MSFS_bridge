// #############################################################################
// ##                                                                         ##
// ## FlightSimBridge.cpp                      (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################
#include "FlightSimBridge.hpp"

// #############################################################################
FlightSimBridge::FlightSimBridge() {}

// #############################################################################
FlightSimBridge::~FlightSimBridge() { disconnect(); }

// #############################################################################
void FlightSimBridge::run() {
    if (!flightSimClient.isConnected()) {
        flightSimClient.connect();
    }
}

// #############################################################################
bool FlightSimBridge::mapEvent(unsigned int eventId, const char* simEventName) {
    return flightSimClient.mapEvent(eventId, simEventName);
}

// #############################################################################
bool FlightSimBridge::sendEvent(unsigned int eventId, unsigned int data) {
    return flightSimClient.sendEvent(eventId, data);
}

// #############################################################################
bool FlightSimBridge::connect() {
    return flightSimClient.connect();
}

// #############################################################################
void FlightSimBridge::disconnect() {
    flightSimClient.disconnect();
}

// #############################################################################
bool FlightSimBridge::isConnected() const {
    return flightSimClient.isConnected();
}

// #############################################################################
unsigned int FlightSimBridge::readCom1Freq() {
    return flightSimClient.readCom1Freq();
}

// #############################################################################
unsigned int FlightSimBridge::readNav1Freq() {
    return flightSimClient.readNav1Freq();
}
