# Find a suitable OpenGL ES 3.0 implementation on this system, i.e.
# - Standard OpenGL libraries (MESA) on Linux
# - ANGLE on Windows
# - Default -lGL on Emscripten

add_library(BoydOpenGL INTERFACE)

if(NOT EMSCRIPTEN)
    if(WIN32)
    	message("GLES: Targetting ANGLE")
        find_package(ANGLE REQUIRED)
        target_include_directories(BoydOpenGL INTERFACE ${ANGLE_INCLUDE_DIRECTORIES})
        target_link_libraries(BoydOpenGL INTERFACE ${ANGLE_EGL_LIBRARY} ${ANGLE_GLESv2_LIBRARY})
    else()
        message("GLES: Targetting platform-specific OpenGL libraries")
        find_package(OpenGL REQUIRED)
        target_link_libraries(BoydOpenGL INTERFACE OpenGL::GL)
    endif()
else()
    message("GLES: Targetting Emscripten")
    target_link_libraries(BoydOpenGL INTERFACE -lGL)

    # USE_WEBGL2=1 enables the web-friendly subset of OpenGL ES 3.0!
    target_compile_options(BoydOpenGL INTERFACE
        "SHELL:-s USE_WEBGL2=1"
    )
    target_link_options(BoydOpenGL INTERFACE
        "SHELL:-s USE_WEBGL2=1"
    )
endif()
