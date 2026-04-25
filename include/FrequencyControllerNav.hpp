#pragma once
#include "FrequencyController.hpp"

class FrequencyControllerNav : public FrequencyController {
protected:
    const Config::RadioConfig& getConfig() const override;
    // ... NAV-specific logic ...
};
