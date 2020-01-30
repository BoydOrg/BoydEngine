# Enable hot reloading by default for Debug builds
option(BOYD_HOT_RELOADING "Enable hot reloading" ON)

if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Debug)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(BOYD_HOT_RELOADING OFF)
endif()

if (BOYD_HOT_RELOADING)
    target_compile_definitions(BoydEngine
        PUBLIC -DBOYD_HOT_RELOADING
    )
endif()

macro(boyd_module module_name priority_no sources)
    list(APPEND BOYD_MODULES "BOYD_MODULE(${module_name}, ${priority_no})")
    add_library(${module_name} MODULE ${sources})

    set_target_properties(${module_name} PROPERTIES
        CXX_VISIBILITY_PRESET hidden
    )
    target_link_libraries(${module_name} PUBLIC BoydEngine)
    target_include_directories(${module_name} PUBLIC $<TARGET_FILE_DIR:BoydEngine>) 
    
    set_target_properties(${module_name} PROPERTIES
        CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
        CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
        CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
    )
endmacro()

boyd_module(Audio 20 Audio/Audio.cc)
boyd_module(Gfx 99 Gfx/Gfx.cc)

message("Test: ${BOYD_MODULES}")

set(BOYD_MODULES_MACRO "")
foreach(BOYD_MODULE ${BOYD_MODULES})
    set(BOYD_MODULES_MACRO "${BOYD_MODULES_MACRO} \\\n ${BOYD_MODULE}")
endforeach()

set(BOYD_MODULES_MACRO ${BOYD_MODULES_MACRO} PARENT_SCOPE)