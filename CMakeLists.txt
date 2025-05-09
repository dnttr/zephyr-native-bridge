cmake_minimum_required(VERSION 3.31)
project(Zephyr-Native-Bridge LANGUAGES C CXX)

#------------------------
# User defined variables.
#------------------------

set(PLATFORM "darwin")
set(ARCHITECTURE "aarch64")

set(TARGET_DIR "${CMAKE_SOURCE_DIR}/targets/${PLATFORM}/${ARCHITECTURE}")

#------------------------
# Build variables.
#------------------------

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#------------------------
# Prepare build.
#------------------------

add_custom_command(
        OUTPUT ${TARGET_DIR}/libznb.dylib
        COMMAND cargo build --release #release? no idea
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/rust
        COMMENT "Building Rust library")

add_custom_target(libznb ALL DEPENDS ${TARGET_DIR}/libznb.dylib)

add_subdirectory(cpp)