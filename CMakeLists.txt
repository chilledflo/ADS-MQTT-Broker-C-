cmake_minimum_required(VERSION 3.20)
project(ADS-Realtime-CPP VERSION 1.0.0 LANGUAGES CXX)

# C++17 für harte Echtzeit-Features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimization flags für minimale Latenz
if(MSVC)
    add_compile_options(/O2 /GL /arch:AVX2 /fp:fast /utf-8)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG /OPT:REF /OPT:ICF")
else()
    add_compile_options(-O3 -march=native -mtune=native -flto -ffast-math)
endif()

# Option für Build ohne TwinCAT ADS (z.B. für CI/CD)
option(BUILD_WITHOUT_ADS "Build without TwinCAT ADS support (CI/CD mode)" OFF)

if(NOT BUILD_WITHOUT_ADS)
    # TwinCAT ADS Library - Prüfe lokale lib/ zuerst, dann System
    if(NOT DEFINED TWINCAT_ADS_ROOT)
        # Versuche zuerst lokales lib-Verzeichnis (für GitHub Actions)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/TcAdsDll.lib")
            set(TWINCAT_ADS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lib")
            message(STATUS "Using bundled TwinCAT ADS library from: ${TWINCAT_ADS_ROOT}")
        else()
            # Fallback auf System-Installation
            set(TWINCAT_ADS_ROOT "C:/Program Files (x86)/Beckhoff/TwinCAT/AdsApi/TcAdsDll")
            message(STATUS "Using system TwinCAT ADS library from: ${TWINCAT_ADS_ROOT}")
        endif()
    endif()
    
    if(WIN32)
        # Windows: TcAdsDll
        find_library(ADS_LIBRARY 
            NAMES TcAdsDll
            PATHS "${TWINCAT_ADS_ROOT}" "${TWINCAT_ADS_ROOT}/Lib/x64"
            REQUIRED
        )
        add_compile_definitions(HAS_TWINCAT_ADS)
    else()
        # Linux: Beckhoff ADS Library (from vcpkg or system)
        find_library(ADS_LIBRARY
            NAMES AdsLib adslib ads
            PATHS /usr/local/lib /usr/lib
            REQUIRED
        )
        find_path(ADS_INCLUDE_DIR
            NAMES AdsLib.h
            PATHS /usr/local/include /usr/include /usr/local/include/AdsLib
            REQUIRED
        )
        include_directories(${ADS_INCLUDE_DIR})
        add_compile_definitions(HAS_ADSLIB)
    endif()
else()
    message(STATUS "Building without ADS support (CI/CD mode)")
endif()

# Paho MQTT C++
find_package(PahoMqttCpp REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(NOT BUILD_WITHOUT_ADS)
    include_directories("${TWINCAT_ADS_ROOT}/Include")
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/ads_realtime_engine.cpp
    src/mqtt_publisher.cpp
    src/plc_discovery.cpp
)

set(HEADERS
    include/ads_realtime_engine.hpp
    include/mqtt_publisher.hpp
    include/realtime_config.hpp
    include/binary_payload.hpp
    include/variable_batch.hpp
    include/shared_memory.hpp
    include/payload_compression.hpp
    include/compressed_payload.hpp
    include/rtss_integration.hpp
    include/linux_rt_preempt.hpp
    include/plc_discovery.hpp
)

# Main executable
add_executable(ads-realtime-bridge ${SOURCES} ${HEADERS})

# Optional: RTSS Example (Windows only)
if(WIN32)
    add_executable(rtss_example examples/rtss_example.cpp)
    target_link_libraries(rtss_example PRIVATE)
endif()

# Optional: Linux RT_PREEMPT Example (Linux only)
if(UNIX AND NOT APPLE)
    add_executable(linux_rt_example examples/linux_rt_example.cpp)
    target_link_libraries(linux_rt_example PRIVATE pthread)
endif()

# Link libraries
if(NOT BUILD_WITHOUT_ADS)
    if(WIN32)
        target_link_libraries(ads-realtime-bridge
            PRIVATE
            ${ADS_LIBRARY}
            PahoMqttCpp::paho-mqttpp3
            ws2_32  # Windows Sockets)
    else()
        # Linux: ADS Library + pthread
        target_link_libraries(ads-realtime-bridge
            PRIVATE
            ${ADS_LIBRARY}
            PahoMqttCpp::paho-mqttpp3
            pthread)
    endif()
else()
    # Build without ADS support
    if(WIN32)
        target_link_libraries(ads-realtime-bridge
            PRIVATE
            PahoMqttCpp::paho-mqttpp3
            ws2_32)
    else()
        target_link_libraries(ads-realtime-bridge
            PRIVATE
            PahoMqttCpp::paho-mqttpp3
            pthread)
    endif()
endif()

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


