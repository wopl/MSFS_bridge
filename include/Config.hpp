// #############################################################################
// ##                                                                         ##
// ## Config.hpp                               (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################
#pragma once
#include <string>
#include "EventTypes.hpp"

namespace Config {

    // UDP
    constexpr int UDP_PORT = 49000;

    // COM1 Radio Commands
    constexpr const char* CMD_COM1_STBY_FINE_DOWN_833 = "sim/radios/stby_com1_fine_down_833";
    constexpr const char* CMD_COM1_STBY_FINE_UP_833 = "sim/radios/stby_com1_fine_up_833";
    constexpr const char* CMD_COM1_STBY_COARSE_DOWN_833 = "sim/radios/stby_com1_coarse_down_833";
    constexpr const char* CMD_COM1_STBY_COARSE_UP_833 = "sim/radios/stby_com1_coarse_up_833";
    constexpr const char* CMD_COM1_FLIP = "sim/radios/com1_standy_flip";
    constexpr const char* CMD_COM1_FREQ_UP = "sim/radios/com1_freq_up_833";

    // NAV1 Radio Commands
    constexpr const char* CMD_NAV1_STBY_FINE_DOWN = "sim/radios/stby_nav1_fine_down";
    constexpr const char* CMD_NAV1_STBY_FINE_UP = "sim/radios/stby_nav1_fine_up";
    constexpr const char* CMD_NAV1_STBY_COARSE_DOWN = "sim/radios/stby_nav1_coarse_down";
    constexpr const char* CMD_NAV1_STBY_COARSE_UP = "sim/radios/stby_nav1_coarse_up";
    constexpr const char* CMD_NAV1_FLIP = "sim/radios/nav1_standy_flip";


    // Frequency config struct (C++20 designated initializers for readability)
    struct RadioConfig {
        const char* name;
        int coarseMin;
        int coarseMax;
        int fineMin;
        int fineMax;
        int fineStep;

        // Command strings
        const char* cmdFineUp;
        const char* cmdFineDown;
        const char* cmdCoarseUp;
        const char* cmdCoarseDown;
        const char* cmdFlip;

        // Event names
        const char* eventSetHz;
        const char* eventSwap;
        const char* eventRequestFreq;
        EventType eventRequestType;
        unsigned int eventSetHzId;
        unsigned int eventSwapId;
        unsigned int eventRequestFreqId;

        // Event types for frequency increment/decrement (radio-specific)
        EventType eventFineUp;
        EventType eventFineDown;
        EventType eventCoarseUp;
        EventType eventCoarseDown;
    };

    // COM1 and NAV1 config instances (C++20 designated initializers)
    constexpr RadioConfig COM1_CONFIG = {
        .name = "COM1",
        .coarseMin = 118,
        .coarseMax = 136,
        .fineMin = 0,
        .fineMax = 995,
        .fineStep = 5,
        .cmdFineUp = CMD_COM1_STBY_FINE_UP_833,
        .cmdFineDown = CMD_COM1_STBY_FINE_DOWN_833,
        .cmdCoarseUp = CMD_COM1_STBY_COARSE_UP_833,
        .cmdCoarseDown = CMD_COM1_STBY_COARSE_DOWN_833,
        .cmdFlip = CMD_COM1_FLIP,
        .eventSetHz = "COM_STBY_RADIO_SET_HZ",
        .eventSwap = "COM_STBY_RADIO_SWAP",
        .eventRequestFreq = "COM1_FREQ_REQUEST",
        .eventRequestType = EventType::REQUEST_COM1_FREQ,
        .eventSetHzId = 0x00011010,
        .eventSwapId = 0x00011015,
        .eventRequestFreqId = 0x00012000,
        .eventFineUp = EventType::COM1_FREQ_FINE_UP,
        .eventFineDown = EventType::COM1_FREQ_FINE_DOWN,
        .eventCoarseUp = EventType::COM1_FREQ_COARSE_UP,
        .eventCoarseDown = EventType::COM1_FREQ_COARSE_DOWN
    };
    constexpr RadioConfig NAV1_CONFIG = {
        .name = "NAV1",
        .coarseMin = 108,
        .coarseMax = 117,
        .fineMin = 0,
        .fineMax = 950,
        .fineStep = 10,
        .cmdFineUp = CMD_NAV1_STBY_FINE_UP,
        .cmdFineDown = CMD_NAV1_STBY_FINE_DOWN,
        .cmdCoarseUp = CMD_NAV1_STBY_COARSE_UP,
        .cmdCoarseDown = CMD_NAV1_STBY_COARSE_DOWN,
        .cmdFlip = CMD_NAV1_FLIP,
        .eventSetHz = "NAV1_STBY_SET",
        .eventSwap = "NAV1_STBY_RADIO_SWAP",
        .eventRequestFreq = nullptr,
        .eventRequestType = EventType::REQUEST_NAV1_FREQ,
        .eventSetHzId = 0x00012010,
        .eventSwapId = 0x00012015,
        .eventRequestFreqId = 0,
        .eventFineUp = EventType::NAV1_FREQ_FINE_UP,
        .eventFineDown = EventType::NAV1_FREQ_FINE_DOWN,
        .eventCoarseUp = EventType::NAV1_FREQ_COARSE_UP,
        .eventCoarseDown = EventType::NAV1_FREQ_COARSE_DOWN
    };

    // Add more configuration constants as needed
}
