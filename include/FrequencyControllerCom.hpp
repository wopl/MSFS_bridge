#pragma once
#include "FrequencyController.hpp"

class FrequencyControllerCom : public FrequencyController {
protected:
    const Config::RadioConfig& getConfig() const override;
    unsigned int readStandbyFreqFromBridge(FlightSimBridge& bridge) const override;
    // ... COM-specific logic ...
};
