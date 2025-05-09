cmake_minimum_required(VERSION 3.31.6)
project(znb LANGUAGES C CXX)

#------------------------
# Build configuration
#------------------------

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_SYSTEM_VERSION 1)

# Include platform-specific and dependency setup
include("${CMAKE_SOURCE_DIR}/cpp/Platform.cmake")
include("${CMAKE_SOURCE_DIR}/cpp/Dependencies.cmake")

# ------------------------
# Define Source Files
# ------------------------
set(SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/cpp/src/main.cpp
)

# ------------------------
# Platform and Build Type
# ------------------------

define_platform()

# Set build type and Rust profile based on CMAKE_BUILD_TYPE
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_TYPE_DIR "debug")
    set(CARGO_PROFILE "dev")
else()
    set(BUILD_TYPE_DIR "release")
    set(CARGO_PROFILE "release")
endif()

# ------------------------
# Create Shared Library
# ------------------------

add_library(znb SHARED ${SOURCE_FILES})

# Include directories
target_include_directories(znb PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Prepare project dependencies
prepare_dependencies()

# ------------------------
# Set Rust Library Output Location
# ------------------------

set(RUST_LIB_NAME "lib${PROJECT_NAME}.${OUTPUT_EXTENSION}")
set(RUST_BUILD_OUTPUT "${CMAKE_SOURCE_DIR}/rust/target/${BUILD_TYPE_DIR}/${RUST_LIB_NAME}")
set(RUST_LIB_TARGET_PATH "${CMAKE_BINARY_DIR}/${BUILD_TYPE_DIR}/${RUST_LIB_NAME}")

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${BUILD_TYPE_DIR}")

# ------------------------
# Build Rust Library
# ------------------------

add_custom_command(
        OUTPUT ${RUST_LIB_TARGET_PATH}
        COMMAND cargo build --manifest-path=${CMAKE_SOURCE_DIR}/rust/Cargo.toml --profile ${CARGO_PROFILE}
        COMMAND ${CMAKE_COMMAND} -E copy ${RUST_BUILD_OUTPUT} ${RUST_LIB_TARGET_PATH}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/rust
        COMMENT "Building and copying Rust library"
)

# ------------------------
# Define Custom Target for Rust Build
# ------------------------

add_custom_target(rust_znb DEPENDS ${RUST_LIB_TARGET_PATH})

# ------------------------
# Link Rust Library to znb
# ------------------------

add_dependencies(znb rust_znb) # Ensure znb depends on rust_znb
target_link_libraries(znb PRIVATE ${RUST_LIB_TARGET_PATH})

# ------------------------
# Create and Link Executable
# ------------------------

add_executable(znb_test ${SOURCE_FILES})
add_dependencies(znb_test rust_znb) # Ensure znb_test depends on rust_znb
target_link_libraries(znb_test PRIVATE ${RUST_LIB_TARGET_PATH})

