file(GLOB_RECURSE JVMTI_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(zephyr-jvmti STATIC ${JVMTI_SOURCES})

target_include_directories(zephyr-jvmti
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(zephyr-jvmti)

configure_zephyr_target(zephyr-jvmti)