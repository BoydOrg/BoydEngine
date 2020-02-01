add_executable(BoydEngine WIN32
    Main.cc
    Core/GameState.cc
    Core/SceneManager.cc
    Modules/Loader.cc
)
set_target_properties(BoydEngine PROPERTIES
    DEFINE_SYMBOL BOYD_DLL_EXPORTS
    ENABLE_EXPORTS YES
)

add_subdirectory(3rdparty/)

target_link_libraries(${PROJECT_NAME} PUBLIC
    EnTT::EnTT
    raylib
    fmt::fmt
    glm
    OpenAL
    LuaBridge
    ${BOYD_LUA}
    polyvox
)

# Generate BoydBuildConfig.hh by compiling & running BoydBuildUtil
add_executable(BoydBuildUtil BoydBuildUtil.cc)
add_custom_target(runBoydBuildUtil
    COMMAND $<TARGET_FILE:BoydBuildUtil> ${CMAKE_CURRENT_BINARY_DIR}/BoydBuildConfig.hh
    COMMENT "Run BoydBuildUtil"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_dependencies(runBoydBuildUtil BoydBuildUtil)
add_dependencies(BoydEngine runBoydBuildUtil)

target_include_directories(BoydEngine PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
)

# Compile all modules
set(BOYD_MODULES "" PARENT_SCOPE)
set(BOYD_MODULES_MACRO "" PARENT_SCOPE)
add_subdirectory(Modules/)

# Then configure BoydEngine.hh via CMake
configure_file(BoydEngine.hh.in BoydEngine.hh)
