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

void FrequencyController::increaseNav1Coarse() {
	const auto& cfg = getConfig();
	nav1_freq_coarse++;
	if (nav1_freq_coarse > cfg.COARSE_MAX) nav1_freq_coarse = cfg.COARSE_MIN;
}

void FrequencyController::decreaseNav1Coarse() {
	const auto& cfg = getConfig();
	nav1_freq_coarse--;
	if (nav1_freq_coarse < cfg.COARSE_MIN) nav1_freq_coarse = cfg.COARSE_MAX;
}

void FrequencyController::increaseNav1Fine() {
	const auto& cfg = getConfig();
	nav1_freq_fine += cfg.FINE_STEP;
	if (nav1_freq_fine > cfg.FINE_MAX) nav1_freq_fine = cfg.FINE_MIN;
}

void FrequencyController::decreaseNav1Fine() {
	const auto& cfg = getConfig();
	nav1_freq_fine -= cfg.FINE_STEP;
	if (nav1_freq_fine < cfg.FINE_MIN) nav1_freq_fine = cfg.FINE_MAX;
}
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
void FrequencyController::setFreqFromHz(unsigned int freq_hz) {
	freq_coarse = static_cast<int>(freq_hz / 1000000);
	int remainder = static_cast<int>(freq_hz % 1000000);
	// Round down to nearest 5 kHz step
	freq_fine = (remainder / 1000) / 5 * 5;
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
void FrequencyController::refreshFreqFromCockpitIfNeeded() {
	std::lock_guard<std::recursive_mutex> lock(freqMutex);
	using namespace std::chrono;
	auto now = steady_clock::now();
	bool needRead = firstFreqEvent || (duration_cast<seconds>(now - lastFreqUpdate).count() > 30);
	if (needRead && bridge && bridge->isConnected()) {
		int before_coarse = freq_coarse;
		int before_fine = freq_fine;
		unsigned int cockpitFreq = bridge->readCom1Freq();
		if (cockpitFreq != 0) {
			setFreqFromHz(cockpitFreq);
			std::ostringstream oss;
			oss << std::fixed << std::setprecision(3);
			oss << "[FREQ-COCKPIT] Overwriting local state from cockpit: "
				<< "Before: coarse=" << before_coarse << ", fine=" << before_fine
				<< ", Cockpit: " << (static_cast<double>(cockpitFreq) / 1e6) << " MHz (" << cockpitFreq << " Hz)"
				<< ", After: coarse=" << freq_coarse << ", fine=" << freq_fine;
			Logger::log(oss.str());
		} else {
			Logger::log("[FREQ] Failed to read COM1 frequency from cockpit", Logger::Level::Warning);
		}
		lastFreqUpdate = now;
		firstFreqEvent = false;
	}
}

// #############################################################################
MsfsEvent FrequencyController::requestCom1Frequency() {
	std::lock_guard<std::recursive_mutex> lock(freqMutex);
	MsfsEvent evt;
	evt.type = EventType::REQUEST_COM1_FREQ;
	evt.name = "Request COM1 Frequency";
	evt.simEventName = "COM1_FREQ_REQUEST";
	auto idIt = msfsEventNameToId.find(evt.simEventName);
	evt.eventId = (idIt != msfsEventNameToId.end()) ? idIt->second : 0;
	evt.data = 0;
	if (bridge && bridge->isConnected()) {
		unsigned int freq = bridge->readCom1Freq();
		setFreqFromHz(freq);
		evt.data = static_cast<unsigned int>((freq_coarse * 1000000) + (freq_fine * 1000));
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(3);
		oss << "[FREQ] Requested COM1 frequency from cockpit: " << (static_cast<double>(evt.data) / 1e6) << " MHz (" << evt.data << " Hz)";
		Logger::log(oss.str());
		lastFreqUpdate = std::chrono::steady_clock::now();
	} else {
		Logger::log("[FREQ] Bridge not connected, cannot request COM1 frequency", Logger::Level::Warning);
	}
	return evt;
}

// #############################################################################
FrequencyController::FrequencyController()
	: freq_coarse(124), freq_fine(0), nav1_freq_coarse(110), nav1_freq_fine(0) {
	std::lock_guard<std::recursive_mutex> lock(freqMutex);
	// Initialization log removed
}
// #############################################################################
// NAV1: Set frequency state from Hz value (splits into coarse/fine)
void FrequencyController::setNav1FreqFromHz(unsigned int freq_hz) {
	nav1_freq_coarse = static_cast<int>(freq_hz / 1000000);
	int remainder = static_cast<int>(freq_hz % 1000000);
	nav1_freq_fine = (remainder / 1000) / 10 * 10;
}

// #############################################################################
bool FrequencyController::shouldRequestNav1Update() const {
	std::lock_guard<std::recursive_mutex> lock(freqMutex);
	using namespace std::chrono;
	auto now = steady_clock::now();
	return firstNav1FreqEvent || (duration_cast<seconds>(now - lastNav1FreqUpdate).count() > 30);
}

// #############################################################################
void FrequencyController::refreshNav1FreqFromCockpitIfNeeded() {
	std::lock_guard<std::recursive_mutex> lock(freqMutex);
	using namespace std::chrono;
	auto now = steady_clock::now();
	bool needRead = firstNav1FreqEvent || (duration_cast<seconds>(now - lastNav1FreqUpdate).count() > 30);
	if (needRead && bridge && bridge->isConnected()) {
		int before_coarse = nav1_freq_coarse;
		int before_fine = nav1_freq_fine;
		unsigned int cockpitFreq = bridge->readNav1Freq();
		if (cockpitFreq != 0) {
			setNav1FreqFromHz(cockpitFreq);
			std::ostringstream oss;
			oss << std::fixed << std::setprecision(3);
			oss << "[NAV1-FREQ-COCKPIT] Overwriting local state from cockpit: "
				<< "Before: coarse=" << before_coarse << ", fine=" << before_fine
				<< ", Cockpit: " << (static_cast<double>(cockpitFreq) / 1e6) << " MHz (" << cockpitFreq << " Hz)"
				<< ", After: coarse=" << nav1_freq_coarse << ", fine=" << nav1_freq_fine;
			Logger::log(oss.str());
		} else {
			Logger::log("[NAV1-FREQ] Failed to read NAV1 frequency from cockpit", Logger::Level::Warning);
		}
		lastNav1FreqUpdate = now;
		firstNav1FreqEvent = false;
	}
}

// #############################################################################
MsfsEvent FrequencyController::requestNav1Frequency() {
	std::lock_guard<std::recursive_mutex> lock(freqMutex);
	MsfsEvent evt;
	evt.type = EventType::REQUEST_NAV1_FREQ;
	evt.name = "Request NAV1 Frequency";
	evt.simEventName = "NAV1_FREQ_REQUEST";
	auto idIt = msfsEventNameToId.find(evt.simEventName);
	evt.eventId = (idIt != msfsEventNameToId.end()) ? idIt->second : 0;
	evt.data = 0;
	if (bridge && bridge->isConnected()) {
		unsigned int freq = bridge->readNav1Freq();
		setNav1FreqFromHz(freq);
		evt.data = static_cast<unsigned int>((nav1_freq_coarse * 1000000) + (nav1_freq_fine * 1000));
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(3);
		oss << "[NAV1-FREQ] Requested NAV1 frequency from cockpit: " << (static_cast<double>(evt.data) / 1e6) << " MHz (" << evt.data << " Hz)";
		Logger::log(oss.str());
		lastNav1FreqUpdate = std::chrono::steady_clock::now();
	} else {
		Logger::log("[NAV1-FREQ] Bridge not connected, cannot request NAV1 frequency", Logger::Level::Warning);
	}
	return evt;
}


// #############################################################################
unsigned int FrequencyController::getCurrentNav1FreqHz() const {
	std::lock_guard<std::recursive_mutex> lock(freqMutex);
	return static_cast<unsigned int>((nav1_freq_coarse * 1000000) + (nav1_freq_fine * 1000));
}

// #############################################################################
unsigned int FrequencyController::fetchNav1FreqNonBlocking() {
	std::lock_guard<std::recursive_mutex> lock(freqMutex);
	unsigned int freq = 0;
	if (bridge && bridge->isConnected()) {
		freq = bridge->readNav1Freq();
		if (freq != 0) {
			setNav1FreqFromHz(freq);
			std::ostringstream oss;
			oss << std::fixed << std::setprecision(3);
			oss << "[NAV1-ASYNC] Non-blocking fetch: NAV1 frequency updated to " << (static_cast<double>(freq) / 1e6) << " MHz (" << freq << " Hz)";
			Logger::log(oss.str());
			lastNav1FreqUpdate = std::chrono::steady_clock::now();
			firstNav1FreqEvent = false;
		} else {
			Logger::log("[NAV1-ASYNC] Non-blocking fetch failed to read NAV1 frequency", Logger::Level::Warning);
		}
	}
	return static_cast<unsigned int>((nav1_freq_coarse * 1000000) + (nav1_freq_fine * 1000));
}

// #############################################################################
MsfsEvent FrequencyController::createNav1FrequencyEvent(EventType type) {
	std::lock_guard<std::recursive_mutex> lock(freqMutex);
	MsfsEvent evt;
	evt.type = type;
	for (const auto& entry : eventRegistry) {
		if (entry.type == type) {
			evt.simEventName = entry.msfsEventName;
			break;
		}
	}
	int before_coarse = nav1_freq_coarse;
	int before_fine = nav1_freq_fine;
	switch (type) {
		case EventType::NAV1_FREQ_FINE_UP:
			evt.name = "NAV1 Frequency (FINE_UP)";
			increaseNav1Fine();
			break;
		case EventType::NAV1_FREQ_FINE_DOWN:
			evt.name = "NAV1 Frequency (FINE_DOWN)";
			decreaseNav1Fine();
			break;
		case EventType::NAV1_FREQ_COARSE_UP:
			evt.name = "NAV1 Frequency (COARSE_UP)";
			increaseNav1Coarse();
			break;
		case EventType::NAV1_FREQ_COARSE_DOWN:
			evt.name = "NAV1 Frequency (COARSE_DOWN)";
			decreaseNav1Coarse();
			break;
		default:
			Logger::log("[NAV1-FREQ] Unknown NAV1 frequency EventType");
			break;
	}
	int after_coarse = nav1_freq_coarse;
	int after_fine = nav1_freq_fine;
	// Set eventId for NAV1 frequency events
	auto idIt = msfsEventNameToId.find(evt.simEventName);
	evt.eventId = (idIt != msfsEventNameToId.end()) ? idIt->second : 0;
	// Use Hz as before (not BCD)
	evt.data = static_cast<unsigned int>((nav1_freq_coarse * 1000000) + (nav1_freq_fine * 1000));
	std::ostringstream oss;
	oss << "[NAV1-FREQ-DEBUG] Event: " << evt.name
		<< ", Before: coarse=" << before_coarse << ", fine=" << before_fine
		<< ", After: coarse=" << after_coarse << ", fine=" << after_fine
		<< ", Data: " << evt.data << ", EventId: 0x" << std::hex << evt.eventId << std::dec;
	Logger::log(oss.str());
	return evt;
}

// #############################################################################
MsfsEvent FrequencyController::createNav1FlipEvent() {
	std::lock_guard<std::recursive_mutex> lock(freqMutex);
	MsfsEvent evt;
	evt.type = EventType::NAV1_STBY_FLIP;
	evt.name = "NAV1 Standby Flip";
	for (const auto& entry : eventRegistry) {
		if (entry.type == EventType::NAV1_STBY_FLIP) {
			evt.simEventName = entry.msfsEventName;
			break;
		}
	}
	auto idIt = msfsEventNameToId.find(evt.simEventName);
	evt.eventId = (idIt != msfsEventNameToId.end()) ? idIt->second : 0;
	evt.data = 0;
	Logger::log("[NAV1-FREQ] Created NAV1 Standby Flip event");
	return evt;
}
// --- Frequency step helpers ---
// #############################################################################
void FrequencyController::increaseCoarse() {
	const auto& cfg = getConfig();
	freq_coarse++;
	if (freq_coarse > cfg.COARSE_MAX) freq_coarse = cfg.COARSE_MIN;
}

void FrequencyController::decreaseCoarse() {
	const auto& cfg = getConfig();
	freq_coarse--;
	if (freq_coarse < cfg.COARSE_MIN) freq_coarse = cfg.COARSE_MAX;
}

void FrequencyController::increaseFine() {
	const auto& cfg = getConfig();
	freq_fine += cfg.FINE_STEP;
	if (freq_fine > cfg.FINE_MAX) freq_fine = cfg.FINE_MIN;
}

void FrequencyController::decreaseFine() {
	const auto& cfg = getConfig();
	freq_fine -= cfg.FINE_STEP;
	if (freq_fine < cfg.FINE_MIN) freq_fine = cfg.FINE_MAX;
}

// #############################################################################
// setFreqFromHz is implemented above using freq_coarse and freq_fine only.

// #############################################################################
MsfsEvent FrequencyController::createFrequencyEvent(EventType type) {
	std::lock_guard<std::recursive_mutex> lock(freqMutex);

	// Only operate on local state; cockpit refresh is handled by MSFSController if needed
	MsfsEvent evt;
	evt.type = type;
	// Lookup event name from eventRegistry
	for (const auto& entry : eventRegistry) {
		if (entry.type == type) {
			evt.simEventName = entry.msfsEventName;
			break;
		}
	}
	int before_coarse = freq_coarse;
	int before_fine = freq_fine;
	switch (type) {
		case EventType::COM1_FREQ_FINE_UP:
			evt.name = "COM1 Frequency (FINE_UP)";
			increaseFine();
			break;
		case EventType::COM1_FREQ_FINE_DOWN:
			evt.name = "COM1 Frequency (FINE_DOWN)";
			decreaseFine();
			break;
		case EventType::COM1_FREQ_COARSE_UP:
			evt.name = "COM1 Frequency (COARSE_UP)";
			increaseCoarse();
			break;
		case EventType::COM1_FREQ_COARSE_DOWN:
			evt.name = "COM1 Frequency (COARSE_DOWN)";
			decreaseCoarse();
			break;
		default:
			Logger::log("[FREQ] Unknown frequency EventType");
			break;
	}
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
unsigned int FrequencyController::fetchCom1FreqNonBlocking() {
	std::lock_guard<std::recursive_mutex> lock(freqMutex);
	unsigned int freq = 0;
	if (bridge && bridge->isConnected()) {
		freq = bridge->readCom1Freq();
		if (freq != 0) {
			setFreqFromHz(freq);
			std::ostringstream oss;
			oss << std::fixed << std::setprecision(3);
			oss << "[ASYNC] Non-blocking fetch: COM1 frequency updated to " << (static_cast<double>(freq) / 1e6) << " MHz (" << freq << " Hz)";
			Logger::log(oss.str());
			lastFreqUpdate = std::chrono::steady_clock::now();
			firstFreqEvent = false;
		} else {
			Logger::log("[ASYNC] Non-blocking fetch failed to read COM1 frequency", Logger::Level::Warning);
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
MsfsEvent FrequencyController::createCom1FlipEvent() {
	std::lock_guard<std::recursive_mutex> lock(freqMutex);
	MsfsEvent evt;
	evt.type = EventType::COM1_STBY_FLIP;
	evt.name = "COM1 Standby Flip";
	// Lookup event name from eventRegistry
	for (const auto& entry : eventRegistry) {
		if (entry.type == EventType::COM1_STBY_FLIP) {
			evt.simEventName = entry.msfsEventName;
			break;
		}
	}
	auto idIt = msfsEventNameToId.find(evt.simEventName);
	evt.eventId = (idIt != msfsEventNameToId.end()) ? idIt->second : 0;
	evt.data = 0;
	Logger::log("[FREQ] Created COM1 Standby Flip event");
	return evt;
}
