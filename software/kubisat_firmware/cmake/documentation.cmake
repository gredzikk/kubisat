# Find required packages
find_package(Doxygen)
find_program(DOT_EXECUTABLE dot)
find_program(LATEXMK_EXECUTABLE latexmk)
find_program(PDFLATEX_EXECUTABLE pdflatex)
find_program(MAKE_EXECUTABLE make)

# Documentation configuration
if(DOXYGEN_FOUND)
    set(DOC_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/release/doc")
    set(DOXYGEN_IN "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile")
    set(DOXYGEN_OUT "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
    set(LATEX_OUTPUT_DIR "${DOC_OUTPUT_DIR}/latex")
    set(FINAL_DOC_NAME "doc_${CURRENT_BUILD_NUMBER}.pdf")
    set(FINAL_DOC_PATH "${DOC_OUTPUT_DIR}/${FINAL_DOC_NAME}")

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

    string(REPLACE ";" " " DOXYGEN_EXCLUDE_PATTERNS
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/storage/pico-vfs/* \
         */unity/* \
         */build/* \
         */release/* \
         */lib/comms/LoRa/*")

    if(DOT_EXECUTABLE)
        set(HAVE_DOT YES)
        get_filename_component(DOT_PATH ${DOT_EXECUTABLE} DIRECTORY)
    else()
        set(HAVE_DOT NO)
        message(STATUS "Graphviz not found - documentation will be generated without graphs")
    endif()

    file(MAKE_DIRECTORY ${DOC_OUTPUT_DIR})
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    if(LATEXMK_EXECUTABLE)
        set(LATEX_BUILD_CMD ${LATEXMK_EXECUTABLE} -pdf -interaction=nonstopmode refman.tex)
    elseif(MAKE_EXECUTABLE)
        set(LATEX_BUILD_CMD ${MAKE_EXECUTABLE})
    elseif(PDFLATEX_EXECUTABLE)
        set(LATEX_BUILD_CMD ${PDFLATEX_EXECUTABLE} -interaction=nonstopmode refman.tex)
    else()
        set(LATEX_BUILD_CMD "")
    endif()

    if(LATEX_BUILD_CMD)
        add_custom_target(doc
            COMMAND ${CMAKE_COMMAND} -E env
                "HAVE_DOT=${HAVE_DOT}"
                "DOT_PATH=${DOT_PATH}"
                "DOC_OUTPUT_DIR=${DOC_OUTPUT_DIR}"
                "DOXYGEN_INPUT_DIRS=${DOXYGEN_INPUT_DIRS}"
                "DOXYGEN_EXCLUDE_PATTERNS=${DOXYGEN_EXCLUDE_PATTERNS}"
                ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            COMMAND ${CMAKE_COMMAND} -E chdir ${LATEX_OUTPUT_DIR} ${LATEX_BUILD_CMD}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${LATEX_OUTPUT_DIR}/refman.pdf" "${FINAL_DOC_PATH}"
            COMMAND ${CMAKE_COMMAND} -E rm -rf ${LATEX_OUTPUT_DIR}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating documentation in release/doc"
            VERBATIM
        )
    else()
        add_custom_target(doc
            COMMAND ${CMAKE_COMMAND} -E env
                "HAVE_DOT=${HAVE_DOT}"
                "DOT_PATH=${DOT_PATH}"
                "DOC_OUTPUT_DIR=${DOC_OUTPUT_DIR}"
                "DOXYGEN_INPUT_DIRS=${DOXYGEN_INPUT_DIRS}"
                "DOXYGEN_EXCLUDE_PATTERNS=${DOXYGEN_EXCLUDE_PATTERNS}"
                ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating HTML documentation (LaTeX toolchain not found)"
            VERBATIM
        )
    endif()

    message(STATUS "Doxygen documentation will be generated when 'make doc' is run")
endif()
