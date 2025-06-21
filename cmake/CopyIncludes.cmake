add_custom_target(copy_includes ALL
        COMMENT "Copying all include files to a single directory"
)

foreach(INCLUDE_DIR ${PROJECT_INCLUDES})
    if(EXISTS ${INCLUDE_DIR} AND NOT ${INCLUDE_DIR} MATCHES "^${CMAKE_BINARY_DIR}")
        add_custom_command(
                TARGET copy_includes
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Copying ${INCLUDE_DIR} to ${COLLECTED_INCLUDES_DESTINATION}"
                COMMAND ${CMAKE_COMMAND} -E copy_directory ${INCLUDE_DIR} ${COLLECTED_INCLUDES_DESTINATION}
        )
    endif()
endforeach()

add_custom_command(
        TARGET copy_includes
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "All includes copied to ${COLLECTED_INCLUDES_DESTINATION}"
)