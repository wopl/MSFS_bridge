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
    void increaseFine();
    void decreaseFine();
    void increaseCoarse();
    void decreaseCoarse();
    void processQueue(MSFSController& msfsController);
    unsigned int getCurrentFreq() const;
    // Future: void syncWithCockpit(unsigned int cockpitFreq);
private:
    void queueChange(FreqChangeType type);
    void adjustFine(int direction, MSFSController& msfsController);
    void adjustCoarse(int direction, MSFSController& msfsController);
    void dispatchFreqEvent(const std::string& typeStr, MSFSController& msfsController);
    std::queue<FreqChangeType> freqQueue;
    std::mutex queueMutex;
    unsigned int com1_freq;
};
