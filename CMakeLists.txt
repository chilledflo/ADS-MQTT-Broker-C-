cmake_minimum_required(VERSION 3.20)
project(ADS-Realtime-CPP VERSION 1.0.0 LANGUAGES CXX)

# C++17 für harte Echtzeit-Features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimization flags für minimale Latenz
if(MSVC)
    add_compile_options(/O2 /GL /arch:AVX2 /fp:fast)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG /OPT:REF /OPT:ICF")
else()
    add_compile_options(-O3 -march=native -mtune=native -flto -ffast-math)
endif()

# TwinCAT ADS Library (Windows)
find_library(ADS_LIBRARY 
    NAMES TcAdsDll
    PATHS "C:/TwinCAT/ADS Api/TcAdsDll/x64/lib"
    REQUIRED
)

# Mosquitto C++ Library für MQTT
find_package(PahoMqttCpp REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    "C:/TwinCAT/ADS Api/TcAdsDll/Include"
)

# Source files
set(SOURCES
    src/main.cpp
    src/ads_realtime_engine.cpp
    src/mqtt_publisher.cpp
    src/variable_manager.cpp
    src/performance_monitor.cpp
)

set(HEADERS
    include/ads_realtime_engine.hpp
    include/mqtt_publisher.hpp
    include/variable_manager.hpp
    include/performance_monitor.hpp
    include/realtime_config.hpp
)

# Main executable
add_executable(ads-realtime-bridge ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(ads-realtime-bridge
    PRIVATE
    ${ADS_LIBRARY}
    PahoMqttCpp::paho-mqttpp3
    ws2_32  # Windows Sockets
)

# Set high priority at runtime
set_target_properties(ads-realtime-bridge PROPERTIES
    WIN32_EXECUTABLE FALSE
    LINK_FLAGS "/SUBSYSTEM:CONSOLE /PRIORITY:HIGH"
)

# Installation
install(TARGETS ads-realtime-bridge
    RUNTIME DESTINATION bin
)

# Documentation
add_subdirectory(docs)
