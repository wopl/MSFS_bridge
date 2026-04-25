#pragma once
#include "FrequencyController.hpp"

class FrequencyControllerNav : public FrequencyController {
protected:
    const Config::RadioConfig& getConfig() const override;
    unsigned int readStandbyFreqFromBridge(FlightSimBridge& bridge) const override;
};
