#include "FrequencyControllerCom.hpp"
#include "Config.hpp"

const Config::RadioConfig& FrequencyControllerCom::getConfig() const {
	return Config::COM1_CONFIG;
}
