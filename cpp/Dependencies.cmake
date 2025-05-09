find_package(Java REQUIRED)
find_package(JNI REQUIRED)
find_package(Catch2 REQUIRED)

function(_prepare_test_lib)
    if (NOT Catch2_FOUND)
        message(FATAL_ERROR "Catch2 not found.")
    endif ()

    message(STATUS "Found Catch2.")
    include_directories(${Catch2_INCLUDE_DIRS})

    include(CTest)
    include(Catch)
endfunction()

function(_prepare_jvm_toolset)
    set(JNI_HEADER "jni_h")
    set(JNI_MD_HEADER "jni_md.h")

    if (NOT JAVA_FOUND OR NOT JNI_FOUND)
        message(FATAL_ERROR "Java or JNI not found.")
    endif ()

    find_path(JNI_H_F ${JNI_HEADER} PATHS ${JNI_INCLUDE_DIRS} REQUIRED)
    find_path(JNI_H_F_ ${JNI_MD_HEADER} PATHS ${JNI_INCLUDE_DIRS} REQUIRED)

    if (NOT JNI_H_F OR NOT JNI_H_F_)
        message(FATAL_ERROR "JNI header not found.")
    endif ()

    message(STATUS "Found JNI.")

    set(JNI_PLATFORM_HEADER "${JNI_H_F}/${PLATFORM}")
endfunction()