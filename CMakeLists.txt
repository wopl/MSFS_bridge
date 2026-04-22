# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Force Visual Studio 2026 generator if available
if (CMAKE_GENERATOR MATCHES "Visual Studio")
	message(STATUS "Using generator: ${CMAKE_GENERATOR}")
else()
	set(CMAKE_GENERATOR "Visual Studio 18 2026" CACHE STRING "Force VS2026 generator" FORCE)
endif()
cmake_minimum_required(VERSION 3.10)
project(MSFS_bridge)

set(CMAKE_CXX_STANDARD 17)



# Set this to your FlightSim SDK path!
set(FLIGHTSIM_SDK "C:/MSFS 2024 SDK/SimConnect SDK")
include_directories(${FLIGHTSIM_SDK}/include)
link_directories(${FLIGHTSIM_SDK}/lib/static)


# Add include directory
include_directories(${CMAKE_SOURCE_DIR}/include)


# Gather only FlightSim and main/controller sources
set(SOURCES
	src/FlightSimBridge.cpp
	src/FlightSimClient.cpp
	src/MSFSController.cpp
	src/UdpReceiver.cpp
	src/main.cpp
	src/Logger.cpp
	src/UdpCommandHandler.cpp
	src/Config.cpp
	src/FrequencyController.cpp
)
add_executable(MSFS_bridge ${SOURCES})

# Link FlightSim and additional Windows libraries required by FlightSim
target_link_libraries(MSFS_bridge SimConnect Ws2_32 Shlwapi)

# Always build in Release mode
set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
