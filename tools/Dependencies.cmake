find_package(Java REQUIRED)
find_package(JNI REQUIRED)
find_package(Catch2 REQUIRED)

function(_prepare_test_lib)
    if (NOT Catch2_FOUND)
        message(FATAL_ERROR "Catch2 not found.")
    endif ()

    message(STATUS "Found Catch2.")
    target_include_directories(${PROJECT_NAME}-tests PRIVATE ${Catch2_INCLUDE_DIRS})

    include(CTest)
    include(Catch)
endfunction()

function(_prepare_jvm_toolset)
    set(JNI_HEADER "jni.h")
    set(JNI_MD_HEADER "jni_md.h")

    if (NOT JAVA_FOUND OR NOT JNI_FOUND)
        message(FATAL_ERROR "Java or JNI not found.")
    endif ()

    find_path(JNI_H_PATH ${JNI_HEADER} PATHS ${JNI_INCLUDE_DIRS} REQUIRED)
    find_path(JNI_H_MD_PATH ${JNI_MD_HEADER} PATHS ${JNI_INCLUDE_DIRS} REQUIRED)

    if (NOT JNI_H_PATH OR NOT JNI_H_MD_PATH)
        message(FATAL_ERROR "JNI header not found.")
    endif ()

    message(STATUS "Found JNI.")

    set(JNI_INCLUDE_OS_SPECIFIC "${JNI_H_PATH}/${OS}")

    if (NOT JNI_INCLUDE_OS_SPECIFIC)
        message(FATAL_ERROR "JNI OS specific header not found.")
    endif ()

    # Add JNI include directories to both the main library and tests
    target_include_directories(${PROJECT_NAME} PUBLIC ${JNI_INCLUDE_DIRS} ${JNI_INCLUDE_OS_SPECIFIC} )
    target_include_directories(${PROJECT_NAME}-tests PRIVATE ${JNI_INCLUDE_DIRS} ${JNI_INCLUDE_OS_SPECIFIC})

    # Link znb library with JNI
    target_link_libraries(${PROJECT_NAME} PUBLIC ${JNI_LIBRARIES})
endfunction()

function(look_for_interface)
    set(PATH "${CMAKE_SOURCE_DIR}/external/zephyr-native-interface.jar")

    if (EXISTS ${PATH})
        message(STATUS "ZNI jar file found: ${PATH}")
    else()
        message(FATAL_ERROR "ZNI jar file not found: ${PATH}. Please compile the ZNI library first. (Java 23+ required)")
    endif()

    set(ZNI_DST ${PATH} PARENT_SCOPE)
endfunction()

function(prepare_dependencies)
    set(JNI_LIB ${JNI_LIBRARIES})
    set(CATCH_2_LIB Catch2::Catch2)

    set(LIB_LIST ${CATCH_2_LIB} PARENT_SCOPE)

    _prepare_test_lib()
    _prepare_jvm_toolset()
endfunction()