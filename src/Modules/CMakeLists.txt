add_library(Gfx MODULE Gfx/Gfx.cpp)

set_target_properties(Gfx PROPERTIES
    CXX_VISIBILITY_PRESET hidden
)
target_link_libraries(Gfx PUBLIC BoydEngine)
target_include_directories(Gfx PUBLIC $<TARGET_FILE_DIR:BoydEngine>)