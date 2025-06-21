function(configure_zephyr_target target)
    target_include_directories(
            ${target}
            PUBLIC
            ${COMMON_INCLUDE_DIRS}
            ${CMAKE_SOURCE_DIR}/extern/include
    )

    target_link_libraries(${target} PRIVATE ${COMMON_LIBS})

    if(APPLE AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_target_properties(
                ${target}
                PROPERTIES
                LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
                OUTPUT_NAME "znb"
                INSTALL_RPATH "@loader_path"
        )
    endif()
endfunction()