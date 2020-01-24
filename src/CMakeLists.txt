add_executable(BoydEngine WIN32
    src/Main.cc
)

configure_file(src/BoydEngine.hh.in src/BoydEngine.hh)
target_include_directories(BoydEngine PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
)

add_subdirectory(3rdparty/)
target_link_libraries(BoydEngine PUBLIC
    EnTT::EnTT
)
