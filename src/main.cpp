// #############################################################################
// ##                                                                         ##
// ## main.cpp                                 (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################

#include "MSFSController.hpp"
#include "UdpReceiver.hpp"
#include "UdpCommandHandler.hpp"
#include "Logger.hpp"
#include "Config.hpp"

// #############################################################################
int main() {
    MSFSController controller;
    Logger::log("[MAIN] controller address: " + std::to_string(reinterpret_cast<uintptr_t>(&controller)));
    UdpCommandHandler udpHandler(controller);
    // Start UDP receiver on configured port
    UdpReceiver udp(Config::UDP_PORT, [&udpHandler](const std::string& packet) {
        Logger::log("Received UDP packet (" + std::to_string(packet.size()) + " bytes)");
        udpHandler.handle(packet);
    });
    udp.start();

    controller.run();
    udp.stop();
    return 0;
}
