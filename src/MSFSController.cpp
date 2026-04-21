#include "MSFSController.hpp"
#include <utility>

void MSFSController::queueFreqChange(FreqChangeSource src) {
    std::lock_guard<std::mutex> lock(queueMutex);
    freqQueue.push(FreqChangeRequest{src});
    std::string srcStr = (src == FreqChangeSource::TIMER) ? "TIMER" : "UDP";
    std::cout << "[QUEUE] Pushed FreqChangeRequest from " << srcStr << ". Queue size now: " << freqQueue.size() << std::endl;
}

MSFSController::MSFSController() : bridge(), com1_freq(118000000) {}

void MSFSController::dispatchEvent(const MsfEvent& evt) {
    std::cout << "[DISPATCH] Mapping event: " << evt.simEventName << " (ID: 0x" << std::hex << evt.eventId << ") with data: " << std::dec << evt.data << std::endl;
    bool mapped = bridge.mapEvent(evt.eventId, evt.simEventName.c_str());
    if (!mapped) {
        std::cout << "[DISPATCH] mapEvent failed for " << evt.simEventName << std::endl;
    }
    bool sent = bridge.sendEvent(evt.eventId, evt.data);
    if (sent) {
        std::cout << "[DISPATCH] " << evt.name << " dispatched with data: " << evt.data << std::endl;
    } else {
        std::cout << "[DISPATCH] sendEvent failed for " << evt.simEventName << std::endl;
    }
}

void MSFSController::run() {
    if (!bridge.connect()) return;
    // Example: Set parking brake once at startup
    MsfEvent brakeEvt{"Parking Brake", EVENT_PARK_BRAKES, 1, "PARKING_BRAKES"};
    dispatchEvent(brakeEvt);
    com1_freq = 118000000;
    // auto lastTimer = std::chrono::steady_clock::now(); // [TIMER] Commented out
    while (true) {
        // Timer pushes to queue (commented out)
        // auto now = std::chrono::steady_clock::now();
        // if (std::chrono::duration_cast<std::chrono::seconds>(now - lastTimer).count() >= 3) {
        //     std::cout << "[TIMER] 3s timer event triggered (pushing to queue)" << std::endl;
        //     queueFreqChange(FreqChangeSource::TIMER);
        //     lastTimer = now;
        // }
        // Main thread processes queue
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            while (!freqQueue.empty()) {
                auto req = freqQueue.front();
                freqQueue.pop();
                std::string src = (req.source == FreqChangeSource::TIMER) ? "TIMER" : "UDP";
                std::cout << "[MAIN] Processing COM1 Fine Up event from " << src << std::endl;
                std::cout << "[MAIN] com1_freq before: " << com1_freq << std::endl;
                com1_freq += 5000; // 5 kHz step
                if (com1_freq > 136990000)
                    com1_freq = 118000000;
                std::cout << "[MAIN] com1_freq after: " << com1_freq << std::endl;
                std::cout << "[MAIN] SimConnect connected: " << (bridge.isConnected() ? "YES" : "NO") << std::endl;
                MsfEvent freqEvt{"COM1 Frequency (" + src + ")", EVENT_COM1_STBY_SET, com1_freq, "COM_STBY_RADIO_SET_HZ"};
                dispatchEvent(freqEvt);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}


