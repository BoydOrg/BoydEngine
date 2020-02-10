add_executable(BoydEngine WIN32
    Main.cc
    Core/GameState.cc
    Core/SceneManager.cc # To be removed when the full asset loader is working
    Modules/Loader.cc

    # Add this header here to make sure BoydBuildUtil gets compiled and run
    ${CMAKE_CURRENT_BINARY_DIR}/BoydBuildConfig.hh
)
set_target_properties(BoydEngine PROPERTIES
    DEFINE_SYMBOL BOYD_DLL_EXPORTS
    ENABLE_EXPORTS YES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"
)
target_compile_definitions(BoydEngine PRIVATE
    # NOTE: PRIVATE because when BOYD_DLL_EXPORTS is set for a module it is not set for BoydEngine, and vice versa!
    -DBOYD_DLL_EXPORTS
)

if(NOT EMSCRIPTEN)
    # Need std::thread...
    find_package(Threads REQUIRED)
    add_library(BoydThreads INTERFACE)
    target_link_libraries(BoydThreads INTERFACE ${CMAKE_THREAD_LIBS_INIT})
else()
    add_library(BoydThreads INTERFACE)
    option(BOYD_EMSCRIPTEN_PTHREADS "Enable Emscripten PThreads support?" OFF)
    # NOTE: Configuration copied in BoydEngine link settings below!
    if(BOYD_EMSCRIPTEN_PTHREADS)
        target_compile_options(BoydThreads INTERFACE "SHELL:-s USE_PTHREADS=1")
    endif()
endif()

add_subdirectory(3rdparty/)

target_link_libraries(${PROJECT_NAME} PUBLIC
    EnTT::EnTT
    fmt::fmt
    glm
    polyvox
    BoydThreads
)
if(UNIX AND NOT APPLE)
    # Required by Dll.hh
    target_link_libraries(${PROJECT_NAME} PUBLIC -ldl)
endif()

if(NOT EMSCRIPTEN)
    # Generate BoydBuildConfig.hh by compiling & running BoydBuildUtil
    add_executable(BoydBuildUtil BoydBuildUtil.cc)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/BoydBuildConfig.hh
        DEPENDS $<TARGET_FILE:BoydBuildUtil>
        COMMAND $<TARGET_FILE:BoydBuildUtil> ${CMAKE_CURRENT_BINARY_DIR}/BoydBuildConfig.hh
        COMMENT "Run BoydBuildUtil"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    target_include_directories(BoydEngine PUBLIC
        "${CMAKE_CURRENT_BINARY_DIR}"
    )
else()
    # Can't (for obvious reasons...) run BoydBuildUtil if it was generated as JavaScript by Emscripten
    # -> Need to generate BoydBuildConfig.hh with sane defaults
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/BoydBuildConfig.hh
        "#pragma once\n"
        "#define BoydEngine__FILE__OFFSET 0\n"
    )
endif()

# Compile all modules
set(BOYD_MODULES "" PARENT_SCOPE)
set(BOYD_MODULES_MACRO "" PARENT_SCOPE)
add_subdirectory(Modules/)

if(EMSCRIPTEN)
    include(${PROJECT_SOURCE_DIR}/emscripten/EmUtilities.cmake)

    # SMALL_XHR_CHINKS: See https://github.com/emscripten-core/emscripten/blob/b82e1d81698a2379f989a4aefc1cd107e00cfdf8/src/library_fs.js#L1734
    set(BOYD_EMSCRIPTEN_PRELOADS "assets@/assets;scripts@/scripts;"
        CACHE STRING
        "Paths to all assets (files/dirs) to bundle in BoydEngine.data when compiling for Emscripten.\nFormat: <dir1>@<mountpoint1>;<dir2>@<mountpoint2>;..."
    )
    em_preload_files(BoydEngine ${BOYD_EMSCRIPTEN_PRELOADS})

    target_link_options(BoydEngine PUBLIC
    	"SHELL:-s SMALL_XHR_CHUNKS=1"
	    "SHELL:-s WASM=1"
        --pre-js "${PROJECT_SOURCE_DIR}/emscripten/pre.js"
    )
    if(BOYD_EMSCRIPTEN_PTHREADS)
        target_link_options(BoydEngine PUBLIC "SHELL:-s USE_PTHREADS=1")
    endif()

    set_target_properties(BoydEngine PROPERTIES
        # Make Emscripten generate the startup page
        SUFFIX ".html"
    )

    # The Emscripten filesystem is mounted at /
    set(BOYD_FS_PREFIX "/")
else()
    # On most platforms assets are directly in the binary dir
    set(BOYD_FS_PREFIX "")
endif()

# Finally, configure BoydEngine.hh via CMake
configure_file(BoydEngine.hh.in BoydEngine.hh)
