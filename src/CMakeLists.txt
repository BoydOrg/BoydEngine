add_executable(BoydEngine WIN32
    Main.cc
    Core/GameState.cc
    Core/SceneManager.cc # To be removed when the full asset loader is working
    Core/MusicAssetLoader.cc # Same as above
    Modules/Loader.cc

    # Add this header here to make sure BoydBuildUtil gets compiled and run
    ${CMAKE_CURRENT_BINARY_DIR}/BoydBuildConfig.hh
)
set_target_properties(BoydEngine PROPERTIES
    DEFINE_SYMBOL BOYD_DLL_EXPORTS
    ENABLE_EXPORTS YES
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

# Compile all modules
set(BOYD_MODULES "" PARENT_SCOPE)
set(BOYD_MODULES_MACRO "" PARENT_SCOPE)
add_subdirectory(Modules/)

# Then configure BoydEngine.hh via CMake
configure_file(BoydEngine.hh.in BoydEngine.hh)
