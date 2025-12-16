# Define minimal required version of CMake.
cmake_minimum_required(VERSION "3.25")

# Help IDEs/clangd find correct include paths.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Project definition
project(
	MatterLightSwitch
	VERSION 1.0
	LANGUAGES C CXX ASM
)

# Include the definition of the slc_MatterLightSwitch target,
# which contains the content of the SLC project
include(MatterLightSwitch.cmake)

add_executable(MatterLightSwitch
    # Add additional sources here
)

# Build app-owned MPR121 driver with the same include path set as other app objects.
target_sources(slc PRIVATE
    ../src/Mpr121.cpp
    ../src/IadcButtons.cpp
)

target_include_directories(MatterLightSwitch PUBLIC
    ../include
    ../autogen
    ../autogen/zap-generated
    ../config
)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
    add_custom_target(copy_compile_commands ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_CURRENT_LIST_DIR}/compile_commands.json
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_CURRENT_LIST_DIR}/../compile_commands.json
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_CURRENT_LIST_DIR}/../../compile_commands.json
        BYPRODUCTS
            ${CMAKE_CURRENT_LIST_DIR}/compile_commands.json
            ${CMAKE_CURRENT_LIST_DIR}/../compile_commands.json
            ${CMAKE_CURRENT_LIST_DIR}/../../compile_commands.json
        COMMENT "Copy compile_commands.json for IntelliSense/clangd"
    )
endif()

target_compile_definitions(MatterLightSwitch PUBLIC
    # Add additional macros here
)

target_compile_options(MatterLightSwitch PUBLIC
    # Set additional compiler flags here
)

target_link_options(MatterLightSwitch PUBLIC
    # Set additional linker flags here
)

# Link with the content defined in the SLC project
target_link_libraries(MatterLightSwitch PRIVATE
    slc
)

# Include managed project content if available
include(MatterLightSwitch_project.cmake OPTIONAL RESULT_VARIABLE managed_project)
if(managed_project)
    message(STATUS "Using managed project content from ${managed_project}")
endif()

# Force the gcc linker command
set_target_properties(MatterLightSwitch PROPERTIES LINKER_LANGUAGE C)

# Create .bin, .hex and .s37 artifacts after building the project
add_custom_command(TARGET MatterLightSwitch
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} ${OBJCOPY_SREC_CMD} "$<TARGET_FILE:MatterLightSwitch>" "$<TARGET_FILE_DIR:MatterLightSwitch>/$<TARGET_FILE_BASE_NAME:MatterLightSwitch>.s37"
    COMMAND ${CMAKE_OBJCOPY} ${OBJCOPY_IHEX_CMD} "$<TARGET_FILE:MatterLightSwitch>" "$<TARGET_FILE_DIR:MatterLightSwitch>/$<TARGET_FILE_BASE_NAME:MatterLightSwitch>.hex"
    COMMAND ${CMAKE_OBJCOPY} ${OBJCOPY_BIN_CMD}  "$<TARGET_FILE:MatterLightSwitch>" "$<TARGET_FILE_DIR:MatterLightSwitch>/$<TARGET_FILE_BASE_NAME:MatterLightSwitch>.bin" 
)

# Run post-build pipeline to perform additional post-processing
if(post_build_command)
add_custom_command(TARGET MatterLightSwitch
    POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/..
    COMMAND ${post_build_command}
)
endif()
