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

    // Only map and send the parking brake event
    // Use a SimConnect event ID in the user-defined range (0x00011000 or higher)
    const int EVENT_PARK_BRAKES = 0x00011000;
    HRESULT mapResult = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_PARK_BRAKES, "PARKING_BRAKES");
    if (FAILED(mapResult)) {
        std::cerr << "Failed to map PARKING_BRAKES event. HRESULT: 0x" << std::hex << mapResult << std::endl;
        SimConnect_Close(hSimConnect);
        return 2;
    }

    while (true) {
        static bool brakeToggle = false;
        brakeToggle = !brakeToggle;
        HRESULT brakeEvt = SimConnect_TransmitClientEvent(
            hSimConnect,
            SIMCONNECT_OBJECT_ID_USER,
            EVENT_PARK_BRAKES,
            0,
            SIMCONNECT_GROUP_PRIORITY_HIGHEST,
            SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY
        );
        if (FAILED(brakeEvt))
            std::cerr << "Parking brake event failed: " << brakeEvt << std::endl;
        else
            std::cout << "Parking brake event sent." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    SimConnect_Close(hSimConnect);
    return 0;
}
