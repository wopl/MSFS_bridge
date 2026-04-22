// #############################################################################
// ##                                                                         ##
// ## UdpCommandHandler.cpp                    (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################

#include "UdpCommandHandler.hpp"
#include "EventTypes.hpp"
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
    auto it = udpCommandToEventType.find(cmd);
    if (it != udpCommandToEventType.end()) {
        Logger::log("[UDP-PARSE] Found event mapping, dispatching to controller");
        controller.queueEvent(it->second);
    } else {
        Logger::log("[UDP-TRACE] Command did not match any known MSFS event mapping");
    }
}
