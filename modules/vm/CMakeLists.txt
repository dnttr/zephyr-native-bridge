file(GLOB_RECURSE VM_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(zephyr-vm STATIC ${VM_SOURCES})

target_include_directories(zephyr-vm
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(zephyr-vm)
configure_zephyr_target(zephyr-vm)