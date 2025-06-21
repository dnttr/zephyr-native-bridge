function(configure_tests)
    file(GLOB_RECURSE TEST_LIST
            "${CMAKE_SOURCE_DIR}/tests/src/impl/*.cpp"
    )

    file(GLOB_RECURSE TEST_SETUP_FILES
            "${CMAKE_SOURCE_DIR}/tests/src/setup.cpp"
    )

    set(TEST_FILES ${TEST_SETUP_FILES})

    if(TEST_LIST)
        message(STATUS "Found test files:")
        foreach(file ${TEST_LIST})
            message(STATUS "  ${file}")
        endforeach()
    else()
        message(WARNING "No test files found in test directories")
    endif()

    if(TEST_SETUP_FILES)
        message(STATUS "Found test setup files:")
        foreach(file ${TEST_SETUP_FILES})
            message(STATUS "  ${file}")
        endforeach()
    endif()

    add_executable(${PROJECT_NAME}-tests ${TEST_FILES} ${TEST_LIST})
    enable_testing()

    target_include_directories(${PROJECT_NAME}-tests PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/tests/include
            ${CMAKE_SOURCE_DIR}/include/private
            ${INCLUDE_DIRECTORIES}
    )

    target_compile_definitions(${PROJECT_NAME}-tests PRIVATE
            ZNI_JAR_PATH="${ZNI_DST}")

    target_link_libraries(${PROJECT_NAME}-tests PRIVATE ${PROJECT_NAME} ${COMMON_LIBS})
endfunction()