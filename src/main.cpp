#include "MSFSController.hpp"
#include "UdpReceiver.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

// COM1 Radio Commands
constexpr const char* CMD_COM1_STBY_FINE_DOWN_833 = "sim/radios/stby_com1_fine_down_833";
constexpr const char* CMD_COM1_STBY_FINE_UP_833 = "sim/radios/stby_com1_fine_up_833";
constexpr const char* CMD_COM1_STBY_COARSE_DOWN_833 = "sim/radios/stby_com1_coarse_down_833";
constexpr const char* CMD_COM1_STBY_COARSE_UP_833 = "sim/radios/stby_com1_coarse_up_833";
constexpr const char* CMD_COM1_FLIP = "sim/radios/com1_standy_flip";

constexpr const char* CMD_COM1_FREQ_UP = "sim/radios/com1_freq_up_833";

#include "msfs_events.hpp"

// Helper: parse UDP command and map to event
void handleUdpCommand(const std::string& packet, MSFSController& controller) {
    // Hex dump of raw packet
    std::ostringstream hex;
    for (unsigned char c : packet) {
        hex << std::hex << std::setw(2) << std::setfill('0') << (int)c << ' ';
    }
    std::cout << "[UDP-TRACE] Raw packet hex: " << hex.str() << std::endl;

    std::string cmd;
    // Remove NULL bytes from packet
    for (char c : packet) {
        if (c != '\0') cmd += c;
    }
    std::cout << "[UDP-TRACE] Sanitized cmd: '" << cmd << "' (size: " << cmd.size() << ")\n";
    // Remove 'CMND' prefix if present
    if (cmd.rfind("CMND", 0) == 0) {
        cmd = cmd.substr(4);
        std::cout << "[UDP-TRACE] Stripped 'CMND' prefix, now: '" << cmd << "'\n";
    }
    std::cout << "[ESP32] Received event: '" << cmd << "'\n";
    if (cmd == CMD_COM1_STBY_FINE_UP_833) {
        std::cout << "[ESP32] COM1 Fine Up event requested (queueing for main thread)" << std::endl;
        controller.queueFreqChange(FreqChangeType::FINE_UP);
    } else if (cmd == CMD_COM1_STBY_FINE_DOWN_833) {
        std::cout << "[ESP32] COM1 Fine Down event requested (queueing for main thread)" << std::endl;
        controller.queueFreqChange(FreqChangeType::FINE_DOWN);
    } else if (cmd == CMD_COM1_STBY_COARSE_UP_833) {
        std::cout << "[ESP32] COM1 Coarse Up event requested (queueing for main thread)" << std::endl;
        controller.queueFreqChange(FreqChangeType::COARSE_UP);
    } else if (cmd == CMD_COM1_STBY_COARSE_DOWN_833) {
        std::cout << "[ESP32] COM1 Coarse Down event requested (queueing for main thread)" << std::endl;
        controller.queueFreqChange(FreqChangeType::COARSE_DOWN);
    } else {
        std::cout << "[UDP-TRACE] Command did not match known COM1 commands\n";
    }
    // The following event handlers are kept for future use:
    // else if (cmd == CMD_COM1_FREQ_UP) {
    //     std::cout << "[ESP32] Increase COM1 frequency event triggered" << std::endl;
    //     controller.com1_freq += 5000; // 5 kHz step
    //     if (controller.com1_freq > 136990000)
    //         controller.com1_freq = 118000000;
    //     MsfEvent freqEvt{"COM1 Frequency", EVENT_COM1_STBY_SET, controller.com1_freq, "COM_STBY_RADIO_SET_HZ"};
    //     controller.dispatchEvent(freqEvt);
    // } else if (cmd == CMD_COM1_STBY_FINE_DOWN_833) {
    //     MsfEvent evt{"COM1 Fine Down", EVENT_COM1_STBY_FINE_DOWN, 0, "COM_STBY_RADIO_FINE_DOWN"};
    //     controller.dispatchEvent(evt);
    // } else if (cmd == CMD_COM1_STBY_COARSE_DOWN_833) {
    //     MsfEvent evt{"COM1 Coarse Down", EVENT_COM1_STBY_COARSE_DOWN, 0, "COM_STBY_RADIO_COARSE_DOWN"};
    //     controller.dispatchEvent(evt);
    // } else if (cmd == CMD_COM1_STBY_COARSE_UP_833) {
    //     MsfEvent evt{"COM1 Coarse Up", EVENT_COM1_STBY_COARSE_UP, 0, "COM_STBY_RADIO_COARSE_UP"};
    //     controller.dispatchEvent(evt);
    // } else if (cmd == CMD_COM1_FLIP) {
    //     MsfEvent evt{"COM1 Flip", EVENT_COM1_STBY_FLIP, 0, "COM_STBY_RADIO_SWAP"};
    //     controller.dispatchEvent(evt);
    // } else {
    //     std::cout << "[ESP32] Unknown UDP command: " << cmd << std::endl;
    // }
}

int main() {
    MSFSController controller;
    std::cout << "[MAIN] controller address: " << &controller << std::endl;
    // Start UDP receiver on port 49000
    UdpReceiver udp(49000, [&controller](const std::string& packet) {
        std::cout << "Received UDP packet (" << packet.size() << " bytes)" << std::endl;
        std::cout << "[UDP] controller address: " << &controller << std::endl;
        handleUdpCommand(packet, controller);
    });
    udp.start();

    controller.run();
    udp.stop();
    return 0;
}
