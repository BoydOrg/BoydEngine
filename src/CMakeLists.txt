add_executable(BoydEngine WIN32
    Main.cc
)

configure_file(BoydEngine.hh.in BoydEngine.hh)
target_include_directories(BoydEngine PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
)

add_subdirectory(3rdparty/)
target_link_libraries(BoydEngine PUBLIC
    EnTT::EnTT
)
