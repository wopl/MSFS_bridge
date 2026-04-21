# MSFS_bridge

A minimal C++ bridge application to connect your COM_NAV implementation with Microsoft Flight Simulator 2024 (MSFS2024) using SimConnect.

## Requirements
- Microsoft Flight Simulator 2024
- SimConnect SDK (part of MSFS SDK)
- CMake 3.10+
- C++17 compatible compiler

## Build Instructions
1. Set the environment variable `MSFS_SDK` to your MSFS SDK root directory.
2. Open a terminal in this folder.
3. Run:
   ```
   cmake -B build
   cmake --build build
   ```
4. Run the executable from the `build` folder.

## Next Steps
- Implement communication with your COM_NAV system.
- Map and forward data between COM_NAV and MSFS2024 as needed.

## Firewall Configuration
To allow this application to communicate with MSFS2024 via SimConnect, ensure your firewall allows inbound and outbound connections for both this executable and SimConnect. You may need to add rules for:
- MSFS_bridge.exe
- SimConnect.dll (if used externally)
- The port configured in SimConnect.xml (default: 500)

Refer to the MSFS SDK documentation for more details on SimConnect network configuration and firewall setup.
