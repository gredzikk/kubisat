# Find required packages
find_package(Doxygen)
find_program(DOT_EXECUTABLE dot)

# Documentation configuration
if(DOXYGEN_FOUND)
    # Set documentation paths
    set(DOC_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/release/doc")
    set(DOXYGEN_IN "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile")
    set(DOXYGEN_OUT "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
    set(LATEX_OUTPUT_DIR "${DOC_OUTPUT_DIR}/latex")
    set(FINAL_DOC_NAME "doc_${CURRENT_BUILD_NUMBER}.pdf")
    set(FINAL_DOC_PATH "${DOC_OUTPUT_DIR}/${FINAL_DOC_NAME}")


    # Convert input directories list to Doxygen format
    string(REPLACE ";" " " DOXYGEN_INPUT_DIRS 
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/comms \
         ${CMAKE_CURRENT_SOURCE_DIR}/lib \
         ${CMAKE_CURRENT_SOURCE_DIR}/lib/powerman \
         ${CMAKE_CURRENT_SOURCE_DIR}/lib/storage \
         ${CMAKE_CURRENT_SOURCE_DIR}/lib/sensors \
         ${CMAKE_CURRENT_SOURCE_DIR}/lib/eventman \
         ${CMAKE_CURRENT_SOURCE_DIR}/lib/clock \
         ${CMAKE_CURRENT_SOURCE_DIR}/lib/location \
         ${CMAKE_CURRENT_SOURCE_DIR}")

    # Convert exclude patterns to Doxygen format
    string(REPLACE ";" " " DOXYGEN_EXCLUDE_PATTERNS 
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/storage/pico-vfs/* \
         */unity/* \
         */build/* \
         */release/* \
         */lib/comms/LoRa/*")

    # Check for Graphviz
    if(DOT_EXECUTABLE)
        set(HAVE_DOT YES)
        get_filename_component(DOT_PATH ${DOT_EXECUTABLE} DIRECTORY)
    else()
        set(HAVE_DOT NO)
        message(STATUS "Graphviz not found - documentation will be generated without graphs")
    endif()

    # Create output directory
    file(MAKE_DIRECTORY ${DOC_OUTPUT_DIR})

    # Configure Doxygen file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(doc
        COMMAND ${CMAKE_COMMAND} -E env
            "HAVE_DOT=${HAVE_DOT}"
            "DOT_PATH=${DOT_PATH}"
            "DOC_OUTPUT_DIR=${DOC_OUTPUT_DIR}"
            "DOXYGEN_INPUT_DIRS=${DOXYGEN_INPUT_DIRS}"
            "DOXYGEN_EXCLUDE_PATTERNS=${DOXYGEN_EXCLUDE_PATTERNS}"
            ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating documentation in release/doc"
        VERBATIM
        # Add command to run make.bat after Doxygen
        COMMAND cmd /c ${DOC_OUTPUT_DIR}/latex/make.bat
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${LATEX_OUTPUT_DIR}/refman.pdf" "${FINAL_DOC_PATH}"
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${LATEX_OUTPUT_DIR}


        COMMENT "Building LaTeX documentation"
    )

    # Optionally, add a message to the build output
    message(STATUS "Doxygen documentation will be generated when 'make doc' is run")
endif()