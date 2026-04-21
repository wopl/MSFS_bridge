cmake_minimum_required(VERSION 3.10)
project(MSFS_bridge)

set(CMAKE_CXX_STANDARD 17)

# Set this to your SimConnect SDK path!
set(SIMCONNECT_SDK "C:/MSFS 2024 SDK/SimConnect SDK")
include_directories(${SIMCONNECT_SDK}/include)
link_directories(${SIMCONNECT_SDK}/lib/static)

add_executable(MSFS_bridge main.cpp)

# Link SimConnect and additional Windows libraries required by SimConnect
target_link_libraries(MSFS_bridge SimConnect Ws2_32 Shlwapi)

# Always build in Release mode
set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
