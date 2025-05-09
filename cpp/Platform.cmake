set(SUPPORTED_ARCHITECTURES "arm64" "aarch64" "armv7")
set(SUPPORTED_OPERATING_SYSTEMS "linux" "darwin")

function(_define_architecture)
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" ARCHITECTURE_NAME)

    list(FIND SUPPORTED_ARCHITECTURES ${ARCHITECTURE_NAME} ARCH_INDEX)

    IF (ARCH_INDEX EQUAL -1)
        message(FATAL_ERROR "Unsupported architecture: ${ARCHITECTURE_NAME}. Supported architectures are: ${SUPPORTED_ARCHITECTURES}.")
    endif ()

    set(ARCHITECTURE "${ARCHITECTURE_NAME}" PARENT_SCOPE)
    message(STATUS "Architecture: ${ARCHITECTURE_NAME}")
endfunction()

function(_define_os)
    string(TOLOWER "${CMAKE_SYSTEM_NAME}" OS_NAME)

    list(FIND SUPPORTED_OPERATING_SYSTEMS ${OS_NAME} OS_INDEX)

    IF (OS_INDEX EQUAL -1)
        message(FATAL_ERROR "Unsupported operating system: ${OS_NAME}. Supported operating systems are: ${SUPPORTED_OPERATING_SYSTEMS}.")
    endif ()

    set(OS "${OS_NAME}" PARENT_SCOPE)
    message(STATUS "Operating system: ${OS_NAME}")
endfunction()

function(define_platform)
    _define_architecture()
    _define_os()

    if (APPLE)
        set(OUTPUT_EXTENSION_NAME "dylib")
    elseif(UNIX)
        set(OUTPUT_EXTENSION_NAME "so")
    endif()

    message(STATUS "Output extension: ${OUTPUT_EXTENSION_NAME}")
    set(OUTPUT_EXTENSION "${OUTPUT_EXTENSION_NAME}" PARENT_SCOPE)

    set(OS "${OS}" PARENT_SCOPE)
    set(ARCHITECTURE "${ARCHITECTURE}" PARENT_SCOPE)
endfunction()
