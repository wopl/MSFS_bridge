// #############################################################################
// ##                                                                         ##
// ## UdpCommandHandler.cpp                    (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################

#include "UdpCommandHandler.hpp"
#include "EventMap.hpp"
#include "Logger.hpp"
#include <sstream>
#include <iomanip>

// #############################################################################
UdpCommandHandler::UdpCommandHandler(MSFSController& controller)
    : controller(controller) {}

// #############################################################################
void UdpCommandHandler::handle(const std::string& packet) {
    // Hex dump of raw packet
    std::ostringstream hex;
    for (unsigned char c : packet) {
        hex << std::hex << std::setw(2) << std::setfill('0') << (int)c << ' ';
    }
    Logger::log(std::string("[UDP-TRACE] Raw packet hex: ") + hex.str());

    std::string cmd;
    // Remove NULL bytes from packet
    for (char c : packet) {
        if (c != '\0') cmd += c;
    }
    Logger::log("[UDP-TRACE] Sanitized cmd: '" + cmd + "' (size: " + std::to_string(cmd.size()) + ")");
    // Remove 'CMND' prefix if present
    if (cmd.rfind("CMND", 0) == 0) {
        cmd = cmd.substr(4);
        Logger::log("[UDP-TRACE] Stripped 'CMND' prefix, now: '" + cmd + "'");
    }
    Logger::log("[ESP32] Received event: '" + cmd + "'");
    auto it = udpToMsfsEventMap.find(cmd);
    if (it != udpToMsfsEventMap.end()) {
        Logger::log("[UDP-PARSE] Found event mapping, dispatching to MSFSController");
        const EventDescriptor& desc = it->second;
        MsfEvent evt{desc.msfsEventName, desc.msfsEventId, static_cast<unsigned int>(desc.step), desc.msfsEventName};
        controller.dispatchEvent(evt);
    } else {
        Logger::log("[UDP-TRACE] Command did not match any known MSFS event mapping");
    }
}
