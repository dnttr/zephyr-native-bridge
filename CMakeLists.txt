cmake_minimum_required(VERSION 3.31.6)

project(zephyr-native-bridge LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_SOURCE_DIR}/tools/Platform.cmake)
include(${CMAKE_SOURCE_DIR}/tools/Dependencies.cmake)

add_compile_definitions(DEBUG)

set(MAIN_HEADER_DEST "${CMAKE_BINARY_DIR}/include")
set(TEST_HEADER_DEST "${CMAKE_BINARY_DIR}/test_include")

file(MAKE_DIRECTORY ${MAIN_HEADER_DEST})
file(MAKE_DIRECTORY ${TEST_HEADER_DEST})

set(SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/cpp/main/src/vm_object.cpp
        ${CMAKE_SOURCE_DIR}/cpp/main/src/vm_management.cpp
        ${CMAKE_SOURCE_DIR}/cpp/main/src/jvmti/jvmti_object.cpp
        ${CMAKE_SOURCE_DIR}/cpp/main/src/jni/utils/util.cpp
)

add_library(znb STATIC ${SOURCE_FILES})

target_include_directories(znb
        PUBLIC
        ${CMAKE_SOURCE_DIR}/cpp/main/include
        ${MAIN_HEADER_DEST}
)

add_dependencies(znb copy_main_headers)

set(TEST_FILES
        ${CMAKE_SOURCE_DIR}/cpp/test/src/setup.cpp
)

set(ZNI_DST "${CMAKE_SOURCE_DIR}/external/zephyr-native-interface.jar")

if (EXISTS ${ZNI_DST})
    message(STATUS "ZNI jar file found: ${ZNI_DST}")
else()
    message(FATAL_ERROR "ZNI jar file not found: ${ZNI_DST}. Please compile the ZNI library first. (Java 23+ required)")
endif()

define_platform()

file(GLOB_RECURSE TEST_LIST "${CMAKE_SOURCE_DIR}/cpp/test/src/impl/*.cpp")

if(TEST_LIST)
    message(STATUS "Found test files:")
    foreach(file ${TEST_LIST})
        message(STATUS "  ${file}")
    endforeach()
else()
    message(WARNING "No test files found in ${CMAKE_SOURCE_DIR}/cpp/test/src/impl/")
endif()

file(GLOB_RECURSE MAIN_HEADER_FILES "${CMAKE_SOURCE_DIR}/cpp/main/include/*.hpp")
file(GLOB_RECURSE TEST_HEADER_FILES "${CMAKE_SOURCE_DIR}/cpp/test/include/*.hpp")

add_custom_target(copy_main_headers ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${MAIN_HEADER_DEST}/ZNBKit"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/cpp/main/include"
        "${MAIN_HEADER_DEST}/ZNBKit"
        DEPENDS ${MAIN_HEADER_FILES}
        COMMENT "Copying main headers to build directory"
)

add_custom_target(copy_test_headers ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${TEST_HEADER_DEST}"
        COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR="${CMAKE_SOURCE_DIR}/cpp/test/include"
        -DDEST_DIR="${TEST_HEADER_DEST}"
        -P "${CMAKE_SOURCE_DIR}/tools/Copy.cmake"
        DEPENDS ${TEST_HEADER_FILES}
        COMMENT "Copying test headers to build directory (excluding config.hpp)"
)

add_custom_target(process_config ALL
        COMMAND ${CMAKE_COMMAND} -E echo "Processing config.hpp with ZNI_DST=${ZNI_DST}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${TEST_HEADER_DEST}/ZNBKit"
        COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE="${CMAKE_SOURCE_DIR}/cpp/test/include/ZNBKit/config.hpp"
        -DDEST_FILE="${TEST_HEADER_DEST}/ZNBKit/config.hpp"
        -DZNI_DST="${ZNI_DST}"
        -P "${CMAKE_SOURCE_DIR}/tools/Config.cmake"
        COMMENT "Configuring test config.hpp with ZNI_DST path"
)

add_executable(znb_tests ${TEST_FILES} ${TEST_LIST})
prepare_dependencies()

add_dependencies(process_config copy_test_headers)
add_dependencies(znb_tests copy_main_headers process_config)

target_include_directories(znb_tests
        PRIVATE
        ${MAIN_HEADER_DEST}
        ${TEST_HEADER_DEST}
        ${CMAKE_SOURCE_DIR}/cpp/main/include/private
)

target_link_libraries(znb_tests PUBLIC znb ${LIB_LIST})

enable_testing()

add_test(NAME run_all_tests
        COMMAND ${CMAKE_COMMAND} -E env
        JAVA_TOOLSET=${JAVA_TEST_TOOLSET}
        java -jar ${ZNI_DST} --library-path=${CMAKE_BINARY_DIR}
        && ${CMAKE_BINARY_DIR}/znb_tests)

install(TARGETS znb
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/cpp/main/include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.hpp")