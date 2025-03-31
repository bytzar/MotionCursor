cmake_minimum_required(VERSION 4.0)  # or newer; 4.0 is invalid, latest CMake is 3.x
project(MotionCursor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add SDL3 as a subdirectory (assumed to be at dependencies/SDL3)
add_subdirectory(dependencies/SDL3)

# Include your own headers
include_directories(${CMAKE_SOURCE_DIR}/headers)

# Find all source files
file(GLOB SOURCES "src/*.cpp")

# Windows resource file
if(WIN32)
    set(RESOURCES assets/MotionCursor.rc)
endif()

# Create executable
add_executable(MotionCursor ${SOURCES} ${RESOURCES})

# Link SDL3 target (exported by SDL's CMake)
target_link_libraries(MotionCursor PRIVATE SDL3::SDL3)

# Copy SDL3.dll after build (runtime dependency)
add_custom_command(TARGET MotionCursor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:SDL3::SDL3>"
        "$<TARGET_FILE_DIR:MotionCursor>"
)