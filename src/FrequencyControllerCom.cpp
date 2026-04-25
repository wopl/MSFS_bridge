#include "FrequencyControllerCom.hpp"
#include "Config.hpp"

const Config::RadioConfig& FrequencyControllerCom::getConfig() const {
	return Config::COM1_CONFIG;
}

unsigned int FrequencyControllerCom::readStandbyFreqFromBridge(FlightSimBridge& bridge) const {
	return bridge.readCom1Freq();
}
