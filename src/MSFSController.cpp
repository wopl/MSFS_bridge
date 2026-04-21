#include "MSFSController.hpp"

MSFSController::MSFSController() : bridge(), com1_freq(118000000) {}

void MSFSController::dispatchEvent(const MsfEvent& evt) {
    bridge.mapEvent(evt.eventId, evt.simEventName.c_str());
    if (bridge.sendEvent(evt.eventId, evt.data)) {
        std::cout << evt.name << " dispatched with data: " << evt.data << std::endl;
    }
}

void MSFSController::run() {
    if (!bridge.connect()) return;
    // Example: Set parking brake once at startup
    MsfEvent brakeEvt{"Parking Brake", EVENT_PARK_BRAKES, 1, "PARKING_BRAKES"};
    dispatchEvent(brakeEvt);

    while (true) {
        MsfEvent freqEvt{"COM1 Frequency", EVENT_COM1_STBY_SET, com1_freq, "COM_STBY_RADIO_SET_HZ"};
        dispatchEvent(freqEvt);
        com1_freq += 5000; // 5 kHz step
        if (com1_freq > 136990000)
            com1_freq = 118000000;
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}
