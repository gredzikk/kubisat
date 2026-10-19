# Ground firmware docs (Linux-friendly LaTeX build)
find_package(Doxygen)
find_program(DOT_EXECUTABLE dot)
find_program(MAKE_EXECUTABLE make)

if(DOXYGEN_FOUND)
    set(DOC_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/release/doc")
    set(DOXYGEN_IN "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile")
    if(EXISTS ${DOXYGEN_IN})
        set(DOXYGEN_OUT "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        add_custom_target(doc
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating ground station documentation"
            VERBATIM
        )
    endif()
endif()
