cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(Application)

target_sources(app PRIVATE src/main.cpp)

# External Modules
add_subdirectory(components/Aled)
add_subdirectory(components/DeviceStatus)
add_subdirectory(components/TAS5825M)