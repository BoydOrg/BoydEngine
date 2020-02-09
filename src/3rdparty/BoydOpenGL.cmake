# Find a suitable OpenGL ES 3.0 implementation on this system, i.e.
# - Standard OpenGL libraries (MESA) on Linux
# - ANGLE on Windows
# - Default -lGL on Emscripten

add_library(BoydOpenGL INTERFACE)

if(NOT EMSCRIPTEN)
    if(WIN32)
    	message("GLES: Targetting ANGLE")
        # FIXME IMPLEMENT!
    else()
	message("GLES: Targetting platform-specific OpenGL libraries")
        find_package(OpenGL REQUIRED)
	target_link_libraries(BoydOpenGL INTERFACE OpenGL::GL)
    endif()
else()
    message("GLES: Targetting Emscripten")
    target_link_libraries(BoydOpenGL INTERFACE -lGL)
    target_compile_options(BoydOpenGL INTERFACE "SHELL:-s FULL_ES3=1")
    target_link_options(BoydOpenGL INTERFACE "SHELL:-s FULL_ES3=1")
endif()
