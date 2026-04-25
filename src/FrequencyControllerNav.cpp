#include "FrequencyControllerNav.hpp"
#include "Config.hpp"

const Config::RadioConfig& FrequencyControllerNav::getConfig() const {
	return Config::NAV1_CONFIG;
}
