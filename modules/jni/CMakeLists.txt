file(GLOB_RECURSE JNI_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(zephyr-jni STATIC ${JNI_SOURCES})

target_include_directories(zephyr-jni
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(zephyr-jni)

configure_zephyr_target(zephyr-jni)