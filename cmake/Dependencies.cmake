find_package(Java REQUIRED)
find_package(JNI REQUIRED)
find_package(Catch2 REQUIRED)

function(configure_tests_library)
    if (NOT Catch2_FOUND)
        message(FATAL_ERROR "Catch2 not found.")
    endif ()

    message(STATUS "Found Catch2.")

    include(CTest)
    include(Catch)
endfunction()

function(configure_jni_library)
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

    set(JNI_SPECIFIC_INCLUDES
            ${JNI_INCLUDE_DIRS}
            ${JNI_OS_SPECIFIC_INCLUDE_DIR}
            PARENT_SCOPE
    )
endfunction()

function(configure_zni_library)
    set(PATH "${CMAKE_SOURCE_DIR}/extern/zephyr-native-interface.jar")

    if (EXISTS ${PATH})
        message(STATUS "ZNI jar file found: ${PATH}")
    else()
        message(FATAL_ERROR "ZNI jar file not found: ${PATH}. Please compile the ZNI library first. (Java 23+ required)")
    endif()

    set(ZNI_VAR ${PATH} PARENT_SCOPE)
endfunction()

configure_jni_library()
configure_tests_library()
configure_zni_library()

set(ZNI_DST ${ZNI_VAR})

set(COMMON_INCLUDE_DIRS
        ${JNI_SPECIFIC_INCLUDES}
        ${CMAKE_BINARY_DIR}
)

set(COMMON_LIBS
        ${JNI_LIBRARIES}
        Catch2::Catch2
)