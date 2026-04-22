// #############################################################################
// ##                                                                         ##
// ## EventTypes.hpp                              (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################
#pragma once
#include <string>

// Enum for all event types
enum class EventType {
    COM1_FREQ_FINE_UP,
    COM1_FREQ_FINE_DOWN,
    COM1_FREQ_COARSE_UP,
    COM1_FREQ_COARSE_DOWN,
    AUTOPILOT_ON,
    AUTOPILOT_OFF,
    BRAKE_SET,
    BRAKE_RELEASE,
    // Add more as needed
};

// Struct for a generic MSFS event
struct MsfEvent {
    EventType type;
    std::string name;
    unsigned int eventId;
    unsigned int data;
    std::string simEventName;
};

// Normalized event registry entry with direct MSFS event name mapping
struct EventRegistryEntry {
    const char* udpCommand;
    EventType type;
    const char* msfsEventName;
};

static const EventRegistryEntry eventRegistry[] = {
    {"sim/radios/stby_com1_fine_up_833",   EventType::COM1_FREQ_FINE_UP,   "COM_STBY_RADIO_SET_HZ"},
    {"sim/radios/stby_com1_fine_down_833", EventType::COM1_FREQ_FINE_DOWN, "COM_STBY_RADIO_SET_HZ"},
    {"sim/radios/stby_com1_coarse_up_833", EventType::COM1_FREQ_COARSE_UP, "COM_STBY_RADIO_SET_HZ"},
    {"sim/radios/stby_com1_coarse_down_833", EventType::COM1_FREQ_COARSE_DOWN, "COM_STBY_RADIO_SET_HZ"},
    // Add more as needed
};

// Central MSFS event info mapping for each event name
struct MsfsEventInfo {
    const char* msfsEventName;
    unsigned int msfsEventId;
};

static const MsfsEventInfo msfsEventInfoMap[] = {
    {"COM_STBY_RADIO_SET_HZ", 0x00011010},
    // Add more as needed for other event names/IDs
};

