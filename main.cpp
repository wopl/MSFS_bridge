// SimConnect event IDs
enum EVENT_ID {
    EVENT_COM1_STBY_SET,
    EVENT_NAV1_STBY_SET,
    EVENT_COM1_SWAP
};

#include <windows.h>
#include <SimConnect.h>
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    HANDLE hSimConnect = nullptr;
    if (FAILED(SimConnect_Open(&hSimConnect, "MSFS Bridge", nullptr, 0, 0, 0))) {
        std::cerr << "Failed to connect to MSFS2024." << std::endl;
        return 1;
    }
    std::cout << "Connected to MSFS2024 via SimConnect!" << std::endl;

    // --- Parking brake code kept for future use ---
    // const int EVENT_PARK_BRAKES = 0x00011000;
    // HRESULT mapResult = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_PARK_BRAKES, "PARKING_BRAKES");
    // if (FAILED(mapResult)) {
    //     std::cerr << "Failed to map PARKING_BRAKES event. HRESULT: 0x" << std::hex << mapResult << std::endl;
    //     SimConnect_Close(hSimConnect);
    //     return 2;
    // }

    // --- COM1 frequency increment every 3 seconds ---
    const int EVENT_COM1_STBY_SET = 0x00011010;
    HRESULT mapCom1 = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_COM1_STBY_SET, "COM_STBY_RADIO_SET_HZ");
    if (FAILED(mapCom1)) {
        std::cerr << "Failed to map COM1 event. HRESULT: 0x" << std::hex << mapCom1 << std::endl;
        SimConnect_Close(hSimConnect);
        return 2;
    }

    DWORD com1_freq = 118000000; // 118.000 MHz in Hz (SimConnect expects Hz*100)
    while (true) {
        HRESULT com1Evt = SimConnect_TransmitClientEvent(
            hSimConnect,
            SIMCONNECT_OBJECT_ID_USER,
            EVENT_COM1_STBY_SET,
            com1_freq,
            SIMCONNECT_GROUP_PRIORITY_HIGHEST,
            SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY
        );
        if (FAILED(com1Evt))
            std::cerr << "COM1 freq set failed: " << com1Evt << std::endl;
        else
            std::cout << "COM1 standby freq set to: " << (com1_freq / 1000000.0) << " MHz" << std::endl;
        com1_freq += 5000; // Increase by 0.005 MHz (5 kHz)
        if (com1_freq > 136990000) // 136.990 MHz
            com1_freq = 118000000;
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    SimConnect_Close(hSimConnect);
    return 0;
}
