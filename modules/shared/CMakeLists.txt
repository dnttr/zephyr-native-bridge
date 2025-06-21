file(GLOB_RECURSE SHARED_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(zephyr-shared STATIC ${SHARED_SOURCES})

target_include_directories(zephyr-shared
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(zephyr-shared)

configure_zephyr_target(zephyr-shared)