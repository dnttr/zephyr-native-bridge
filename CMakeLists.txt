cmake_minimum_required(VERSION 3.31.6)

project(zephyr-native-bridge LANGUAGES C CXX)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-diagnostics-show-template-tree")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-backtrace-limit=0")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_SOURCE_DIR}/cmake/Platform.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/Dependencies.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/TargetConfig.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CopyIncludes.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/Test.cmake)

file(GLOB_RECURSE ALL_SOURCES
        "${CMAKE_SOURCE_DIR}/modules/jni/src/*.cpp"
        "${CMAKE_SOURCE_DIR}/modules/jvmti/src/*.cpp"
        "${CMAKE_SOURCE_DIR}/modules/shared/src/*.cpp"
        "${CMAKE_SOURCE_DIR}/modules/vm/src/*.cpp"
)

add_library(${PROJECT_NAME} SHARED ${ALL_SOURCES})

set(PROJECT_INCLUDES
        ${CMAKE_SOURCE_DIR}/modules/vm/include
        ${CMAKE_SOURCE_DIR}/modules/jni/include
        ${CMAKE_SOURCE_DIR}/modules/jvmti/include
        ${CMAKE_SOURCE_DIR}/modules/shared/include
)

set(COLLECTED_INCLUDES
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/extern/include
        ${COMMON_INCLUDE_DIRS}
        ${PROJECT_INCLUDES}
)


set(COLLECTED_INCLUDES_DESTINATION "${CMAKE_BINARY_DIR}/include")

target_include_directories(${PROJECT_NAME} PUBLIC ${COLLECTED_INCLUDES} ${PROJECT_INCLUDES})
target_link_libraries(${PROJECT_NAME} PRIVATE ${COMMON_LIBS})

configure_tests()

file(MAKE_DIRECTORY ${COLLECTED_INCLUDES_DESTINATION})
copy_includes()

set_target_properties(${PROJECT_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        OUTPUT_NAME "znb"
        INSTALL_RPATH "@loader_path"
)