# CMakeLists.txt
cmake_minimum_required(VERSION 3.31.6)
project(znb LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include platform and dependencies
include(${CMAKE_SOURCE_DIR}/tools/Platform.cmake)
include(${CMAKE_SOURCE_DIR}/tools/Dependencies.cmake)

# Source files
set(SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/cpp/src/main.cpp
)

set(TEST_FILES
        ${CMAKE_SOURCE_DIR}/cpp/test/setup.cpp
        ${CMAKE_SOURCE_DIR}/cpp/test/test_main.cpp
)

set(ZNI_DST "${CMAKE_SOURCE_DIR}/external/zephyr-native-interface.jar")

if (EXISTS ${ZNI_DST})
    message(STATUS "ZNI jar file found: ${ZNI_DST}")
else()
    message(FATAL_ERROR "ZNI jar file not found: ${ZNI_DST}. Please compile the ZNI library first. (Java 23+ required)")
endif ()

configure_file(
        ${CMAKE_SOURCE_DIR}/cpp/test/setup.hpp
        ${CMAKE_BINARY_DIR}/cpp/test/setup.hpp
        @ONLY
)

define_platform()  # from Platform.cmake

# Rust build settings
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(RUST_PROFILE dev)
    set(RUST_BUILD_DIR debug)
else()
    set(RUST_PROFILE release)
    set(RUST_BUILD_DIR release)
endif()

set(RUST_LIB_NAME "lib${PROJECT_NAME}.${OUTPUT_EXTENSION}")
set(RUST_OUT       "${CMAKE_SOURCE_DIR}/rust/target/${RUST_BUILD_DIR}/${RUST_LIB_NAME}")
set(RUST_DST       "${CMAKE_BINARY_DIR}/${RUST_BUILD_DIR}/${RUST_LIB_NAME}")

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${RUST_BUILD_DIR}")

# Build Rust staticlib
add_custom_command(
        OUTPUT ${RUST_DST}
        COMMAND cargo build --manifest-path=${CMAKE_SOURCE_DIR}/rust/Cargo.toml --profile ${RUST_PROFILE}
        COMMAND ${CMAKE_COMMAND} -E copy ${RUST_OUT} ${RUST_DST}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/rust
        COMMENT "Building Rust static library"
)
add_custom_target(rust_znb DEPENDS ${RUST_DST})

# Collect test sources
file(GLOB_RECURSE TEST_SOURCES "${CMAKE_SOURCE_DIR}/cpp/tests/*.cpp")

# Create Catch2-based test executable
add_executable(znb_tests ${TEST_FILES})
prepare_dependencies()
add_dependencies(znb_tests rust_znb)
target_include_directories(znb_tests PUBLIC
        ${CMAKE_BINARY_DIR}
        ${CMAKE_SOURCE_DIR}/cpp/include
)
target_link_libraries(znb_tests PRIVATE ${LIB_LIST} ${RUST_DST})

# Enable and register testing
enable_testing()
add_test(NAME run_all_tests
        COMMAND ${CMAKE_COMMAND} -E env
        JAVA_TOOLSET=${JAVA_TEST_TOOLSET}
        java -jar ${ZNI_DST} --library-path=${CMAKE_BINARY_DIR}/${RUST_BUILD_DIR}
        && ${CMAKE_BINARY_DIR}/znb_tests)