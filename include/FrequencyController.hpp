// #############################################################################
// ##                                                                         ##
// ## FrequencyController.hpp                  (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################
#pragma once
#include <queue>
#include <mutex>
#include "MSFSController.hpp"
#include "Config.hpp"

class FrequencyController {
public:
    FrequencyController();
    void queueChange(FreqChangeType type);
    void processQueue(MSFSController& msfsController);
    unsigned int getCurrentFreq() const;
private:
    std::queue<FreqChangeType> freqQueue;
    std::mutex queueMutex;
    unsigned int com1_freq;
};
