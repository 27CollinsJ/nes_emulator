cmake_minimum_required(VERSION 4.1)
project(nes_emulator)

set(CMAKE_CXX_STANDARD 26)

file(GLOB MODULE_FILES CONFIGURE_DEPENDS "*.cppm")

add_executable(nes_emulator main.cpp)

target_sources(nes_emulator PUBLIC
    FILE_SET CXX_MODULES FILES
        ${MODULE_FILES}
)
