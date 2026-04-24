// #############################################################################
// ##                                                                         ##
// ## EventTypes.hpp                           (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################
#pragma once
#include <string>
#include <unordered_map>
#include <chrono>

// Enum for all event types
enum class EventType {
    COM1_FREQ_FINE_UP,
    COM1_FREQ_FINE_DOWN,
    COM1_FREQ_COARSE_UP,
    COM1_FREQ_COARSE_DOWN,
    COM1_STBY_FLIP, // New event type for standby flip
    REQUEST_COM1_FREQ, // New event type for frequency request
    AUTOPILOT_ON,
    AUTOPILOT_OFF,
    BRAKE_SET,
    BRAKE_RELEASE,
    // Add more as needed
};

// Async event state for queue management
enum class MsfsEventState {
    Ready,
    PendingInstrumentUpdate,
    FailedTimeout
};

// Struct for a generic MSFS event
struct MsfsEvent {
    EventType type;
    std::string name;
    unsigned int eventId;
    unsigned int data;
    std::string simEventName;
    MsfsEventState state = MsfsEventState::Ready;
    // For pending events, track instrument type and request time
    std::string instrumentKey; // e.g., "COM1", "NAV1"
    std::chrono::steady_clock::time_point requestTime;
};

// Normalized event registry entry with direct MSFS event name mapping
struct EventRegistryEntry {
    EventType type;
    const char* msfsEventName;
};

static const EventRegistryEntry eventRegistry[] = {
    {EventType::COM1_FREQ_FINE_UP,   "COM_STBY_RADIO_SET_HZ"},
    {EventType::COM1_FREQ_FINE_DOWN, "COM_STBY_RADIO_SET_HZ"},
    {EventType::COM1_FREQ_COARSE_UP, "COM_STBY_RADIO_SET_HZ"},
    {EventType::COM1_FREQ_COARSE_DOWN, "COM_STBY_RADIO_SET_HZ"},
    {EventType::COM1_STBY_FLIP,      "COM_STBY_RADIO_SWAP"}, // Standard SimConnect event for flip
    {EventType::REQUEST_COM1_FREQ,   "COM1_FREQ_REQUEST"}, // New registry entry
    // Add more as needed
};

// O(1) UDP command to EventType lookup
static const std::unordered_map<std::string, EventType> udpCommandToEventType = {
    {"sim/radios/stby_com1_fine_up_833",   EventType::COM1_FREQ_FINE_UP},
    {"sim/radios/stby_com1_fine_down_833", EventType::COM1_FREQ_FINE_DOWN},
    {"sim/radios/stby_com1_coarse_up_833", EventType::COM1_FREQ_COARSE_UP},
    {"sim/radios/stby_com1_coarse_down_833", EventType::COM1_FREQ_COARSE_DOWN},
    {"sim/radios/com1_standy_flip",        EventType::COM1_STBY_FLIP}, // New mapping for flip
    // Add more as needed
};

// O(1) MSFS event name to event ID lookup
static const std::unordered_map<std::string, unsigned int> msfsEventNameToId = {
    {"COM_STBY_RADIO_SET_HZ", 0x00011010},
    {"COM_STBY_RADIO_SWAP", 0x00011015}, // Standard SimConnect event for standby flip
    {"COM1_FREQ_REQUEST", 0x00012000}, // Assign a unique ID for the request event
    // Add more as needed
};

