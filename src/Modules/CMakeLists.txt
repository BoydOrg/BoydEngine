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

macro(boyd_module)
    # boyd_module(
    #   NAME <name>
    #   PRIORITY <priority_no>
    #   SOURCES <source files...>
    #   LINKS <link libraries/targets...>
    #   INCLUDES <include directories...>
    #)
    set(OPTIONS)
    set(ONE_VALUE_ARGS NAME PRIORITY)
    set(MULTI_VALUE_ARGS SOURCES LINKS INCLUDES)
    cmake_parse_arguments(_BOYD_MODULE "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    list(APPEND BOYD_MODULES "BOYD_MODULE(${_BOYD_MODULE_NAME}, ${_BOYD_MODULE_PRIORITY})")

    if(BOYD_HOT_RELOADING)
        add_library(${_BOYD_MODULE_NAME} MODULE ${_BOYD_MODULE_SOURCES})
    else()
        add_library(${_BOYD_MODULE_NAME} STATIC ${_BOYD_MODULE_SOURCES})
    endif()

    set_target_properties(${_BOYD_MODULE_NAME} PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
        CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
        CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
    )
    target_include_directories(${_BOYD_MODULE_NAME} BEFORE PUBLIC
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        ${_BOYD_MODULE_INCLUDES}
    ) 
    target_link_libraries(${_BOYD_MODULE_NAME} PUBLIC
        ${_BOYD_MODULE_LINKS}
    )

    if (BOYD_HOT_RELOADING)
        target_link_libraries(${_BOYD_MODULE_NAME} PUBLIC ${PROJECT_NAME})
    else()
        # Fixes compilation error "BoydBuildConfig.hh not found"
        target_include_directories(${_BOYD_MODULE_NAME} PUBLIC
            "${CMAKE_CURRENT_BINARY_DIR}/../"
        )
        target_link_libraries(${PROJECT_NAME} PUBLIC ${_BOYD_MODULE_NAME})
    endif()
endmacro()

# ------------------------------------------------------------------------------

boyd_module(NAME Scripting PRIORITY 1
    SOURCES Scripting/Scripting.cc Scripting/Registrar.cc
    LINKS LuaBridge ${BOYD_LUA}
)

boyd_module(NAME Audio PRIORITY 20
    SOURCES Audio/Audio.cc Audio/Utils.cc
    LINKS OpenAL
)

boyd_module(NAME Gfx PRIORITY 99
    SOURCES Gfx/Gfx.cc
)

# ------------------------------------------------------------------------------

set(BOYD_MODULES_MACRO "")
foreach(BOYD_MODULE ${BOYD_MODULES})
    set(BOYD_MODULES_MACRO "${BOYD_MODULES_MACRO} \\\n ${BOYD_MODULE}")
endforeach()

set(BOYD_MODULES_MACRO ${BOYD_MODULES_MACRO} PARENT_SCOPE)
