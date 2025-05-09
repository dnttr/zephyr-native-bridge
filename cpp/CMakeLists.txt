add_library(Zephyr-Native-Bridge SHARED ${TARGET_DIR}/libznb.dylib)

target_include_directories(Zephyr-Native-Bridge PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_dependencies(Zephyr-Native-Bridge libznb)