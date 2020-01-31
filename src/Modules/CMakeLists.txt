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
else()
    # allow target_link_libraries to work from the bottom up
    cmake_policy(SET CMP0079 NEW) 
endif()

macro(boyd_module module_name priority_no sources)
    list(APPEND BOYD_MODULES "BOYD_MODULE(${module_name}, ${priority_no})")

    if(BOYD_HOT_RELOADING)
        add_library(${module_name} MODULE ${sources})
    else()
        add_library(${module_name} STATIC ${sources})
    endif()


    set_target_properties(${module_name} PROPERTIES
        CXX_VISIBILITY_PRESET hidden
    )
    
    set_target_properties(${module_name} PROPERTIES
        CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
        CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
        CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
    )

    target_include_directories(${module_name} PUBLIC $<TARGET_FILE_DIR:${PROJECT_NAME}>) 

    if (BOYD_HOT_RELOADING)
        target_link_libraries(${module_name} PUBLIC ${PROJECT_NAME})
    else()
        # Fixes compilation error "BoydBuildConfig.hh not found"
        target_include_directories(${module_name} PUBLIC
            "${CMAKE_CURRENT_BINARY_DIR}/../"
        )
        target_link_libraries(${PROJECT_NAME} PUBLIC ${module_name})
    endif()
endmacro()

boyd_module(Audio 20 Audio/Audio.cc)
boyd_module(Voxel 98 Voxel/Voxel.cc)
boyd_module(Gfx 99 Gfx/Gfx.cc)

target_link_libraries(Audio PUBLIC tinywav FLAC FLAC++)

set(BOYD_MODULES_MACRO "")
foreach(BOYD_MODULE ${BOYD_MODULES})
    set(BOYD_MODULES_MACRO "${BOYD_MODULES_MACRO} \\\n ${BOYD_MODULE}")
endforeach()

set(BOYD_MODULES_MACRO ${BOYD_MODULES_MACRO} PARENT_SCOPE)
