cmake_minimum_required(VERSION 4.0)
project(MotionCursor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add SDL3 as a subdirectory (assumed to be at dependencies/SDL3)
add_subdirectory(dependencies/SDL3)
add_subdirectory(dependencies/cpr)

# Include your own headers
include_directories(${CMAKE_SOURCE_DIR}/headers)

# Find all source files
file(GLOB SOURCES "src/*.cpp")

# Windows resource file
if(WIN32)
    set(RESOURCES assets/MotionCursor.rc)
endif()

############################################################
# Set the path to ImGui
set(IMGUI_PATH ${CMAKE_SOURCE_DIR}/dependencies/imgui)

# Include ImGui headers
include_directories(${IMGUI_PATH})

# Add ImGui source files
file(GLOB IMGUI_SOURCES
    ${IMGUI_PATH}/imgui.cpp
    ${IMGUI_PATH}/imgui_demo.cpp
    ${IMGUI_PATH}/imgui_draw.cpp
    ${IMGUI_PATH}/imgui_tables.cpp
    ${IMGUI_PATH}/imgui_widgets.cpp
    ${IMGUI_PATH}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_PATH}/backends/imgui_impl_sdlrenderer3.cpp
)

# Include the backend headers
include_directories(${IMGUI_PATH}/backends)

# Combine your source files with ImGui sources
set(ALL_SOURCES ${SOURCES} ${IMGUI_SOURCES} ${RESOURCES})

# Create executable
add_executable(MotionCursor WIN32 ${SOURCES} ${IMGUI_SOURCES} ${RESOURCES})

add_compile_definitions(IMGUI_ENABLE_DOCKING)

# Link SDL3 target (exported by SDL's CMake)
target_link_libraries(MotionCursor PRIVATE SDL3::SDL3 cpr::cpr)

# Copy SDL3.dll after build (runtime dependency)
add_custom_command(TARGET MotionCursor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:SDL3::SDL3>"
        "$<TARGET_FILE_DIR:MotionCursor>"
)