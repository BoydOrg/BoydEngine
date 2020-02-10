# Select the files to --preload to create a data package for emscripten.
# TARGET is the executable target.
# Pass each filepath to preload, in `<realpath>@<emscrpath>` format.
function(em_preload_files TARGET DATAFILE)
    # See Emscripten/Modules/Platform/Emscripten.cmake
    set(PRELOAD_ARGS "")
    foreach(PRELOAD ${ARGN})
        set(PRELOAD_ARGS "${PRELOAD_ARGS};--preload;${PRELOAD};")
    endforeach()

    get_target_property(OUT_DIR ${TARGET} RUNTIME_OUTPUT_DIRECTORY)
    get_target_property(OUT_NAME ${TARGET} OUTPUT_NAME)
    set(OUT_DATA_PATH "${OUT_DIR}/${OUT_NAME}.data")
    set(OUT_JS_PATH "${OUT_DIR}/${OUT_NAME}.preload.js")

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
endfunction()
