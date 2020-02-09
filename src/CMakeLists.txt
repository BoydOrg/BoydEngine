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

# Need std::thread...
find_package(Threads REQUIRED)

add_subdirectory(3rdparty/)

target_link_libraries(${PROJECT_NAME} PUBLIC
    EnTT::EnTT
    fmt::fmt
    glm
    polyvox
    ${CMAKE_THREAD_LIBS_INIT}
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

# Then configure BoydEngine.hh via CMake
configure_file(BoydEngine.hh.in BoydEngine.hh)
