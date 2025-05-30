cmake_minimum_required(VERSION 3.31.6)

project(zephyr-native-bridge LANGUAGES C CXX)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-diagnostics-show-template-tree")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU") # For GCC
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-backtrace-limit=0")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_SOURCE_DIR}/tools/Platform.cmake)
include(${CMAKE_SOURCE_DIR}/tools/Dependencies.cmake)

set(MAIN_HEADER_DEST "${CMAKE_BINARY_DIR}/include")
set(TEST_HEADER_DEST "${CMAKE_BINARY_DIR}/test_include")

file(MAKE_DIRECTORY ${MAIN_HEADER_DEST})
file(MAKE_DIRECTORY ${TEST_HEADER_DEST})

file(GLOB_RECURSE MAIN_HEADER_FILES "${CMAKE_SOURCE_DIR}/cpp/main/include/*.hpp")
file(GLOB_RECURSE TEST_HEADER_FILES "${CMAKE_SOURCE_DIR}/cpp/test/include/*.hpp")

file(GLOB_RECURSE TEST_LIST "${CMAKE_SOURCE_DIR}/cpp/test/src/impl/*.cpp")

set(SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/cpp/main/src/vm_object.cpp
        ${CMAKE_SOURCE_DIR}/cpp/main/src/vm_management.cpp
        ${CMAKE_SOURCE_DIR}/cpp/main/src/jvmti/jvmti_object.cpp
        ${CMAKE_SOURCE_DIR}/cpp/main/src/jvmti/jvmti_factory.cpp
        ${CMAKE_SOURCE_DIR}/cpp/main/src/jni/utils/util.cpp
)

set(TEST_FILES
        ${CMAKE_SOURCE_DIR}/cpp/test/src/setup.cpp
)

look_for_interface()
define_platform()

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${CMAKE_SOURCE_DIR}/cpp/main/include
        ${MAIN_HEADER_DEST}
)

add_dependencies(${PROJECT_NAME} copy_main_headers)

if(TEST_LIST)
    message(STATUS "Found test files:")
    foreach(file ${TEST_LIST})
        message(STATUS "  ${file}")
    endforeach()
else()
    message(WARNING "No test files found in ${CMAKE_SOURCE_DIR}/cpp/test/src/impl/")
endif()

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

add_executable(${PROJECT_NAME}-tests ${TEST_FILES} ${TEST_LIST})

prepare_dependencies()
enable_testing()

add_dependencies(process_config copy_test_headers)
add_dependencies(${PROJECT_NAME}-tests copy_main_headers process_config)

target_include_directories(${PROJECT_NAME}-tests
        PRIVATE
        ${MAIN_HEADER_DEST}
        ${TEST_HEADER_DEST}
        ${CMAKE_SOURCE_DIR}/cpp/main/include/private
)

target_link_libraries(${PROJECT_NAME}-tests PUBLIC ${PROJECT_NAME} ${LIB_LIST})

add_test(NAME run_all_tests
        COMMAND ${CMAKE_COMMAND} -E env
        JAVA_TOOLSET=${JAVA_TEST_TOOLSET}
        java -jar ${ZNI_DST} --library-path=${CMAKE_BINARY_DIR}
        && ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-tests)

install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/cpp/main/include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.hpp")

set_target_properties(${PROJECT_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        OUTPUT_NAME "znb"
        INSTALL_RPATH "@loader_path"
)