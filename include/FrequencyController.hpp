// #############################################################################
// ##                                                                         ##
// ## FrequencyController.hpp                  (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################
#pragma once
#include <queue>
#include <mutex>
#include "EventTypes.hpp"
#include "Config.hpp"

class FrequencyController {
public:
    MsfEvent createFrequencyEvent(EventType type);
    FrequencyController();
    unsigned int increaseFine();
    unsigned int decreaseFine();
    unsigned int increaseCoarse();
    unsigned int decreaseCoarse();
    unsigned int getCurrentFreq() const;
    // Future: void syncWithCockpit(unsigned int cockpitFreq);
private:
    unsigned int adjustFine(int direction);
    unsigned int adjustCoarse(int direction);
    unsigned int com1_freq;
};
