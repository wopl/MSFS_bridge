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
    COM1_STBY_FLIP, // COM1 standby flip
    REQUEST_COM1_FREQ, // COM1 frequency request
    // NAV1 events
    NAV1_FREQ_FINE_UP,
    NAV1_FREQ_FINE_DOWN,
    NAV1_FREQ_COARSE_UP,
    NAV1_FREQ_COARSE_DOWN,
    NAV1_STBY_FLIP, // NAV1 standby flip
    REQUEST_NAV1_FREQ, // NAV1 frequency request (if needed)
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
    {EventType::COM1_STBY_FLIP,      "COM_STBY_RADIO_SWAP"},
    {EventType::REQUEST_COM1_FREQ,   "COM1_FREQ_REQUEST"},
    // NAV1 events (all use SET_HZ for freq, SWAP for flip)
    {EventType::NAV1_FREQ_FINE_UP,   "NAV1_STBY_SET"},
    {EventType::NAV1_FREQ_FINE_DOWN, "NAV1_STBY_SET"},
    {EventType::NAV1_FREQ_COARSE_UP, "NAV1_STBY_SET"},
    {EventType::NAV1_FREQ_COARSE_DOWN, "NAV1_STBY_SET"},
    {EventType::NAV1_STBY_FLIP,      "NAV1_STBY_RADIO_SWAP"},
    // Add more as needed
};

// O(1) UDP command to EventType lookup
static const std::unordered_map<std::string, EventType> udpCommandToEventType = {
    {"sim/radios/stby_com1_fine_up_833",   EventType::COM1_FREQ_FINE_UP},
    {"sim/radios/stby_com1_fine_down_833", EventType::COM1_FREQ_FINE_DOWN},
    {"sim/radios/stby_com1_coarse_up_833", EventType::COM1_FREQ_COARSE_UP},
    {"sim/radios/stby_com1_coarse_down_833", EventType::COM1_FREQ_COARSE_DOWN},
    {"sim/radios/com1_standy_flip",        EventType::COM1_STBY_FLIP},
    // NAV1 mappings
    {"sim/radios/stby_nav1_fine_down",     EventType::NAV1_FREQ_FINE_DOWN},
    {"sim/radios/stby_nav1_fine_up",       EventType::NAV1_FREQ_FINE_UP},
    {"sim/radios/stby_nav1_coarse_down",   EventType::NAV1_FREQ_COARSE_DOWN},
    {"sim/radios/stby_nav1_coarse_up",     EventType::NAV1_FREQ_COARSE_UP},
    {"sim/radios/nav1_standy_flip",        EventType::NAV1_STBY_FLIP},
    // Add more as needed
};

// O(1) MSFS event name to event ID lookup
static const std::unordered_map<std::string, unsigned int> msfsEventNameToId = {
    {"COM_STBY_RADIO_SET_HZ", 0x00011010},
    {"COM_STBY_RADIO_SWAP", 0x00011015},
    {"COM1_FREQ_REQUEST", 0x00012000},
    // NAV1 event IDs (match SET and SWAP)
    {"NAV1_STBY_SET", 0x00012010},
    {"NAV1_STBY_RADIO_SWAP", 0x00012015},
    // Add more as needed
};

