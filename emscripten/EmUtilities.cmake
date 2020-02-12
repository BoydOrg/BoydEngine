# Select the files to --preload to create a data package for emscripten.
# TARGET is the executable target.
# FILENAME is the base name of the output files; if falsey, the arguments are passed directly to emcc (--preload-file)
# Pass each filepath to preload, in `<realpath>@<emscrpath>` format.
function(em_preload_files TARGET FILENAME)
    if(FILENAME)
        # Create a .data file manually and add make its loader be --pre-js

        # See Emscripten/Modules/Platform/Emscripten.cmake
        set(PRELOAD_ARGS "")
        foreach(PRELOAD ${ARGN})
            set(PRELOAD_ARGS "${PRELOAD_ARGS};--preload;${PRELOAD};")
        endforeach()

        get_target_property(OUT_DIR ${TARGET} RUNTIME_OUTPUT_DIRECTORY)
        set(OUT_DATA_PATH "${OUT_DIR}/${FILENAME}.data")
        set(OUT_JS_PATH "${OUT_DIR}/${FILENAME}.preload.js")

        # TODO: Only execute this if the files/folders to preload change...
        add_custom_command(
            OUTPUT "${OUT_DATA_PATH}" "${OUT_JS_PATH}"
            COMMAND "python"
            ARGS "${EMSCRIPTEN_ROOT_PATH}/tools/file_packager.py" "${OUT_DATA_PATH}" "--js-output=${OUT_JS_PATH}" ${PRELOAD_ARGS}
            COMMENT "Package Emscripten assets"
        )
        add_custom_target(${TARGET}_empreload
            DEPENDS "${OUT_DATA_PATH}" "${OUT_JS_PATH}"
        ) 
        add_dependencies(${TARGET} ${TARGET}_empreload)

        target_link_options(${TARGET} PUBLIC --pre-js ${OUT_JS_PATH})
    else()
        # Add args directly to emcc
        set(PRELOAD_ARGS "")
        foreach(PRELOAD ${ARGN})
            set(PRELOAD_ARGS ${PRELOAD_ARGS} "SHELL:--preload-file ${PRELOAD}")
        endforeach()
        target_link_options(${TARGET} PUBLIC ${PRELOAD_ARGS})
    endif()

    # Add dependency to asset files so that the file-packager will run again if modified
    set(ALL_ASSETS "")
    foreach(PRELOAD ${ARGN})
        string(REPLACE "@" ";" PRELOAD_PAIR "${PRELOAD}")
        list(GET PRELOAD_PAIR 0 ASSET_PATH)

        if(IS_DIRECTORY "${ASSET_PATH}")
            file(GLOB_RECURSE ASSET_FILES
                FOLLOW_SYMLINKS
                LIST_DIRECTORIES false
                "${ASSET_PATH}/*"
            )
        else()
            set(ASSET_FILES "${ASSET_PATH}")
        endif()
        set(ALL_ASSETS "${ALL_ASSETS};${ASSET_FILES};")
    endforeach()

    add_custom_target(${TARGET}_Assets DEPENDS ${ALL_ASSETS})
    add_dependencies(${TARGET} ${TARGET}_Assets)

    message(VERBOSE "Emscripten: preloads of ${TARGET} are: ${ALL_ASSETS}")

endfunction()
