function(boyd_module module_name sources)
    add_library(${module_name} MODULE ${sources})

    set_target_properties(${module_name} PROPERTIES
        CXX_VISIBILITY_PRESET hidden
    )
    target_link_libraries(${module_name} PUBLIC BoydEngine)
    target_include_directories(${module_name} PUBLIC $<TARGET_FILE_DIR:BoydEngine>)   
endfunction()

boyd_module(Gfx Gfx/Gfx.cpp)