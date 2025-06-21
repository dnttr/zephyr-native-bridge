set(SUPPORTED_ARCHITECTURES "arm64" "aarch64" "armv7")
set(SUPPORTED_OPERATING_SYSTEMS "linux" "darwin")

function(_get_architecture ARCH_VAR)
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" ARCHITECTURE_NAME)

    list(FIND SUPPORTED_ARCHITECTURES ${ARCHITECTURE_NAME} ARCH_INDEX)
    if(ARCH_INDEX EQUAL -1)
        message(
                FATAL_ERROR
                "Unsupported architecture: ${ARCHITECTURE_NAME}. Supported: ${SUPPORTED_ARCHITECTURES}."
        )
    endif()

    set(${ARCH_VAR} "${ARCHITECTURE_NAME}" PARENT_SCOPE)
endfunction()

function(_get_os OS_VAR)
    string(TOLOWER "${CMAKE_SYSTEM_NAME}" OS_NAME)

    list(FIND SUPPORTED_OPERATING_SYSTEMS ${OS_NAME} OS_INDEX)
    if(OS_INDEX EQUAL -1)
        message(
                FATAL_ERROR
                "Unsupported OS: ${OS_NAME}. Supported: ${SUPPORTED_OPERATING_SYSTEMS}."
        )
    endif()

    set(${OS_VAR} "${OS_NAME}" PARENT_SCOPE)
endfunction()

_get_architecture(LOCAL_ARCH)
_get_os(LOCAL_OS)

set(ARCHITECTURE ${LOCAL_ARCH})
set(OS ${LOCAL_OS})

message(STATUS "Architecture: ${ARCHITECTURE}")
message(STATUS "Operating system: ${OS}")