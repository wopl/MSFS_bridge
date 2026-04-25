// #############################################################################
// ##                                                                         ##
// ## FrequencyController.cpp                  (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################
#include "FrequencyController.hpp"
#include "EventTypes.hpp"
#include "FlightSimBridge.hpp"
#include "Logger.hpp"
#include <chrono>
#include <mutex>
#include <sstream>
#include <iomanip>

// #############################################################################
// Helper: extract freq_coarse and freq_fine from Hz
// [REFACTORED] Uses config-driven fineStep instead of hardcoded 5 kHz
void FrequencyController::syncStateFromCockpitHz(unsigned int freq_hz) {
	const auto& cfg = getConfig();
	freq_coarse = static_cast<int>(freq_hz / 1000000);
	int remainder = static_cast<int>(freq_hz % 1000000);
	// Round down to nearest fineStep (e.g., 5 kHz for COM1, may differ for other radios)
	freq_fine = (remainder / 1000) / cfg.fineStep * cfg.fineStep;
}
// #############################################################################
bool FrequencyController::shouldRequestUpdate() const {
	std::lock_guard<std::recursive_mutex> lock(freqMutex);
	using namespace std::chrono;
	auto now = steady_clock::now();
	return firstFreqEvent || (duration_cast<seconds>(now - lastFreqUpdate).count() > 30);
}

// #############################################################################
void FrequencyController::setBridge(FlightSimBridge* bridgePtr) {
	std::lock_guard<std::recursive_mutex> lock(freqMutex);
	bridge = bridgePtr;
}

// #############################################################################
bool FrequencyController::isFrequencyStepEvent(EventType type) const {
	const auto& cfg = getConfig();
	return type == cfg.eventFineUp ||
		type == cfg.eventFineDown ||
		type == cfg.eventCoarseUp ||
		type == cfg.eventCoarseDown;
}

// #############################################################################
bool FrequencyController::isFlipEvent(EventType type) const {
	return type == getConfig().eventFlipType;
}

// #############################################################################
bool FrequencyController::isFrequencyRequestEvent(EventType type) const {
	return type == getConfig().eventRequestType;
}

// #############################################################################
std::string FrequencyController::getInstrumentKey() const {
	const auto& cfg = getConfig();
	return (cfg.name != nullptr) ? cfg.name : "RADIO";
}

// #############################################################################
// [REFACTORED] Generic config-driven, fully abstracted from COM1
void FrequencyController::refreshFreqFromCockpitIfNeeded() {
	std::lock_guard<std::recursive_mutex> lock(freqMutex);
	using namespace std::chrono;
	auto now = steady_clock::now();
	bool needRead = firstFreqEvent || (duration_cast<seconds>(now - lastFreqUpdate).count() > 30);
	if (needRead && bridge && bridge->isConnected()) {
		// Get radio-specific config (COM1 or NAV1 depending on which child we are)
		const auto& cfg = getConfig();
		const std::string radioName = (cfg.name != nullptr) ? cfg.name : "RADIO";
		int before_coarse = freq_coarse;
		int before_fine = freq_fine;
		// Virtual hook: child class provides bridge read (e.g., readCom1Freq or readNav1Freq)
		unsigned int cockpitFreq = readStandbyFreqFromBridge(*bridge);
		if (cockpitFreq != 0) {
			syncStateFromCockpitHz(cockpitFreq);
			std::ostringstream oss;
			oss << std::fixed << std::setprecision(3);
			oss << "[FREQ-COCKPIT] Overwriting local state from cockpit: "
				<< "Before: coarse=" << before_coarse << ", fine=" << before_fine
				<< ", Cockpit: " << (static_cast<double>(cockpitFreq) / 1e6) << " MHz (" << cockpitFreq << " Hz)"
				<< ", After: coarse=" << freq_coarse << ", fine=" << freq_fine;
			Logger::log(oss.str());
		} else {
			Logger::log("[FREQ] Failed to read " + radioName + " frequency from cockpit", Logger::Level::Warning);
		}
		lastFreqUpdate = now;
		firstFreqEvent = false;
	}
}

// #############################################################################
// [REFACTORED] Generic config-driven, fully abstracted from COM1
MsfsEvent FrequencyController::requestFrequency() {
	std::lock_guard<std::recursive_mutex> lock(freqMutex);
	// Get radio-specific config (COM1 or NAV1 depending on which child we are)
	const auto& cfg = getConfig();
	const std::string radioName = (cfg.name != nullptr) ? cfg.name : "RADIO";
	MsfsEvent evt;
	// Use config event type/name instead of hardcoded COM1 constants
	evt.type = cfg.eventRequestType;
	evt.name = "Request " + radioName + " Frequency";
	evt.simEventName = (cfg.eventRequestFreq != nullptr) ? cfg.eventRequestFreq : "";
	evt.eventId = cfg.eventRequestFreqId;
	if (evt.eventId == 0 && !evt.simEventName.empty()) {
		auto idIt = msfsEventNameToId.find(evt.simEventName);
		evt.eventId = (idIt != msfsEventNameToId.end()) ? idIt->second : 0;
	}
	evt.data = 0;
	if (bridge && bridge->isConnected()) {
		// Virtual hook: child class provides bridge read (e.g., readCom1Freq or readNav1Freq)
		unsigned int freq = readStandbyFreqFromBridge(*bridge);
		syncStateFromCockpitHz(freq);
		evt.data = static_cast<unsigned int>((freq_coarse * 1000000) + (freq_fine * 1000));
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(3);
		oss << "[FREQ] Requested " << radioName << " frequency from cockpit: " << (static_cast<double>(evt.data) / 1e6) << " MHz (" << evt.data << " Hz)";
		Logger::log(oss.str());
		lastFreqUpdate = std::chrono::steady_clock::now();
	} else {
		Logger::log("[FREQ] Bridge not connected, cannot request " + radioName + " frequency", Logger::Level::Warning);
	}
	return evt;
}

// #############################################################################
FrequencyController::FrequencyController()
	: freq_coarse(124), freq_fine(0) {
	std::lock_guard<std::recursive_mutex> lock(freqMutex);
}
// --- Frequency step helpers ---
// #############################################################################
void FrequencyController::increaseCoarse() {
	const auto& cfg = getConfig();
	freq_coarse++;
	if (freq_coarse > cfg.coarseMax) freq_coarse = cfg.coarseMin;
}

void FrequencyController::decreaseCoarse() {
	const auto& cfg = getConfig();
	freq_coarse--;
	if (freq_coarse < cfg.coarseMin) freq_coarse = cfg.coarseMax;
}

void FrequencyController::increaseFine() {
	const auto& cfg = getConfig();
	freq_fine += cfg.fineStep;
	if (freq_fine > cfg.fineMax) freq_fine = cfg.fineMin;
}

void FrequencyController::decreaseFine() {
	const auto& cfg = getConfig();
	freq_fine -= cfg.fineStep;
	if (freq_fine < cfg.fineMin) freq_fine = cfg.fineMax;
}

// #############################################################################
// [REFACTORED] Generic config-driven, uses config-stored event types
MsfsEvent FrequencyController::createFrequencyEvent(EventType type) {
	std::lock_guard<std::recursive_mutex> lock(freqMutex);
	const auto& cfg = getConfig();
	const std::string radioName = (cfg.name != nullptr) ? cfg.name : "RADIO";

	// Only operate on local state; cockpit refresh is handled by MSFSController if needed
	MsfsEvent evt;
	evt.type = type;
	evt.simEventName = cfg.eventSetHz;
	evt.eventId = cfg.eventSetHzId;
	int before_coarse = freq_coarse;
	int before_fine = freq_fine;

	// Map incoming EventType to action using config-driven event types.
	std::string actionName;
	if (type == cfg.eventFineUp) {
		actionName = "FINE_UP";
		increaseFine();
	} else if (type == cfg.eventFineDown) {
		actionName = "FINE_DOWN";
		decreaseFine();
	} else if (type == cfg.eventCoarseUp) {
		actionName = "COARSE_UP";
		increaseCoarse();
	} else if (type == cfg.eventCoarseDown) {
		actionName = "COARSE_DOWN";
		decreaseCoarse();
	} else {
		Logger::log("[FREQ] Unknown frequency EventType for " + radioName + "; dropping event", Logger::Level::Warning);
		evt.name = radioName + " Frequency (INVALID_EVENT_TYPE)";
		evt.simEventName.clear();
		evt.eventId = 0;
		evt.data = 0;
		return evt;
	}

	evt.name = radioName + " Frequency (" + actionName + ")";
	int after_coarse = freq_coarse;
	int after_fine = freq_fine;
	evt.data = static_cast<unsigned int>((freq_coarse * 1000000) + (freq_fine * 1000));
	std::ostringstream oss;
	oss << "[FREQ-DEBUG] Event: " << evt.name
		<< ", Before: coarse=" << before_coarse << ", fine=" << before_fine
		<< ", After: coarse=" << after_coarse << ", fine=" << after_fine
		<< ", Data: " << evt.data;
	Logger::log(oss.str());
	return evt;
}

// #############################################################################
// [REFACTORED] Generic config-driven, fully abstracted from COM1
unsigned int FrequencyController::fetchFreqNonBlocking() {
	std::lock_guard<std::recursive_mutex> lock(freqMutex);
	unsigned int freq = 0;
	if (bridge && bridge->isConnected()) {
		// Virtual hook: child class provides bridge read (e.g., readCom1Freq or readNav1Freq)
		freq = readStandbyFreqFromBridge(*bridge);
		if (freq != 0) {
			syncStateFromCockpitHz(freq);
			// Get radio-specific config for logging (COM1, NAV1, etc.)
			const auto& cfg = getConfig();
			const std::string radioName = (cfg.name != nullptr) ? cfg.name : "RADIO";
			std::ostringstream oss;
			oss << std::fixed << std::setprecision(3);
			oss << "[ASYNC] Non-blocking fetch: " << radioName << " frequency updated to " << (static_cast<double>(freq) / 1e6) << " MHz (" << freq << " Hz)";
			Logger::log(oss.str());
			lastFreqUpdate = std::chrono::steady_clock::now();
			firstFreqEvent = false;
		} else {
			const auto& cfg = getConfig();
			const std::string radioName = (cfg.name != nullptr) ? cfg.name : "RADIO";
			Logger::log("[ASYNC] Non-blocking fetch failed to read " + radioName + " frequency", Logger::Level::Warning);
		}
	}
	return static_cast<unsigned int>((freq_coarse * 1000000) + (freq_fine * 1000));
}

// #############################################################################
// Thread-safe getter for current frequency in Hz
unsigned int FrequencyController::getCurrentFreqHz() const {
	std::lock_guard<std::recursive_mutex> lock(freqMutex);
	return static_cast<unsigned int>((freq_coarse * 1000000) + (freq_fine * 1000));
}

// #############################################################################
// [REFACTORED] Generic config-driven, fully abstracted from COM1
MsfsEvent FrequencyController::createFlipEvent() {
	std::lock_guard<std::recursive_mutex> lock(freqMutex);
	// Pull all radio-specific constants from the active child config.
	const auto& cfg = getConfig();
	const std::string radioName = (cfg.name != nullptr) ? cfg.name : "RADIO";
	MsfsEvent evt;
	// Swap event metadata is fully config-driven (no hardcoded COM1).
	evt.simEventName = cfg.eventSwap;
	evt.eventId = cfg.eventSwapId;
	evt.type = cfg.eventFlipType;
	// Keep a readable event label in logs/queue traces.
	evt.name = radioName + " Standby Flip";
	evt.data = 0;
	// After a swap, local standby state is stale. Force next operation to refresh from cockpit.
	firstFreqEvent = true;
	Logger::log("[FREQ] Created " + radioName + " flip event: " + evt.simEventName);
	return evt;
}