#include "MSFSController.hpp"
#include <utility>

void MSFSController::queueFreqChange(FreqChangeType type) {
    std::lock_guard<std::mutex> lock(queueMutex);
    freqQueue.push(FreqChangeRequest{type});
    std::string typeStr;
    switch (type) {
        case FreqChangeType::FINE_UP: typeStr = "FINE_UP"; break;
        case FreqChangeType::FINE_DOWN: typeStr = "FINE_DOWN"; break;
        case FreqChangeType::COARSE_UP: typeStr = "COARSE_UP"; break;
        case FreqChangeType::COARSE_DOWN: typeStr = "COARSE_DOWN"; break;
    }
    std::cout << "[QUEUE] Pushed FreqChangeRequest type " << typeStr << ". Queue size now: " << freqQueue.size() << std::endl;
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
        // Main thread processes queue
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            while (!freqQueue.empty()) {
                auto req = freqQueue.front();
                freqQueue.pop();
                std::string typeStr;
                int step = 0;
                switch (req.type) {
                    case FreqChangeType::FINE_UP:
                        typeStr = "FINE_UP";
                        step = 5000;
                        break;
                    case FreqChangeType::FINE_DOWN:
                        typeStr = "FINE_DOWN";
                        step = -5000;
                        break;
                    case FreqChangeType::COARSE_UP:
                        typeStr = "COARSE_UP";
                        step = 1000000; // 1 MHz
                        break;
                    case FreqChangeType::COARSE_DOWN:
                        typeStr = "COARSE_DOWN";
                        step = -1000000; // 1 MHz
                        break;
                }
                std::cout << "[MAIN] Processing COM1 event type " << typeStr << std::endl;
                std::cout << "[MAIN] com1_freq before: " << com1_freq << std::endl;
                int next_freq = static_cast<int>(com1_freq) + step;
                if (next_freq > 136990000)
                    next_freq = 118000000;
                if (next_freq < 118000000)
                    next_freq = 136990000;
                com1_freq = static_cast<unsigned int>(next_freq);
                std::cout << "[MAIN] com1_freq after: " << com1_freq << std::endl;
                std::cout << "[MAIN] SimConnect connected: " << (bridge.isConnected() ? "YES" : "NO") << std::endl;
                MsfEvent freqEvt{"COM1 Frequency (" + typeStr + ")", EVENT_COM1_STBY_SET, com1_freq, "COM_STBY_RADIO_SET_HZ"};
                dispatchEvent(freqEvt);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}


