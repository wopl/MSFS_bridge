// #############################################################################
// ##                                                                         ##
// ## EventMap.hpp                             (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################

#pragma once
#include <string>
#include <unordered_map>
#include "Config.hpp"

struct EventDescriptor {
    std::string msfsEventName;
    unsigned int msfsEventId;
    int step;
};

// This map links UDP command strings to MSFS event descriptors.
static const std::unordered_map<std::string, EventDescriptor> udpToMsfsEventMap = {
    {Config::CMD_COM1_STBY_FINE_UP_833,   {"COM_STBY_RADIO_SET_HZ", 0x00011010,  Config::COM1_FREQ_FINE_STEP}},
    {Config::CMD_COM1_STBY_FINE_DOWN_833, {"COM_STBY_RADIO_SET_HZ", 0x00011010, -static_cast<int>(Config::COM1_FREQ_FINE_STEP)}},
    {Config::CMD_COM1_STBY_COARSE_UP_833, {"COM_STBY_RADIO_SET_HZ", 0x00011010,  Config::COM1_FREQ_COARSE_STEP}},
    {Config::CMD_COM1_STBY_COARSE_DOWN_833, {"COM_STBY_RADIO_SET_HZ", 0x00011010, -static_cast<int>(Config::COM1_FREQ_COARSE_STEP)}},
    // Add more mappings as needed
};
