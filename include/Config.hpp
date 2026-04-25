// #############################################################################
// ##                                                                         ##
// ## Config.hpp                               (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################
#pragma once
#include "EventTypes.hpp"

namespace Config {

    // UDP
    constexpr int UDP_PORT = 49000;

    // Generic fallback event IDs
    constexpr unsigned int EVENT_PARK_BRAKES_ID = 0x00011000;

    // Frequency config struct (C++20 designated initializers for readability)
    struct RadioConfig {
        const char* name;
        int coarseMin;
        int coarseMax;
        int fineMin;
        int fineMax;
        int fineStep;

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
        EventType eventFlipType;
    };

    // COM1 and NAV1 config instances (C++20 designated initializers)
    constexpr RadioConfig COM1_CONFIG = {
        .name = "COM1",
        .coarseMin = 118,
        .coarseMax = 136,
        .fineMin = 0,
        .fineMax = 995,
        .fineStep = 5,
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
        .eventCoarseDown = EventType::COM1_FREQ_COARSE_DOWN,
        .eventFlipType = EventType::COM1_STBY_FLIP
    };
    constexpr RadioConfig NAV1_CONFIG = {
        .name = "NAV1",
        .coarseMin = 108,
        .coarseMax = 117,
        .fineMin = 0,
        .fineMax = 950,
        .fineStep = 10,
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
        .eventCoarseDown = EventType::NAV1_FREQ_COARSE_DOWN,
        .eventFlipType = EventType::NAV1_STBY_FLIP
    };

    // Add more configuration constants as needed
}
