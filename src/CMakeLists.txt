add_executable(BoydEngine WIN32
    Main.cc
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

# Then configure BoydEngine.hh via CMake
configure_file(BoydEngine.hh.in BoydEngine.hh)
target_include_directories(BoydEngine PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
)

# Compile all modules
add_subdirectory(Modules/)