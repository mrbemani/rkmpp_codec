cmake_minimum_required(VERSION 3.10)

# Set the project name
project(CvToMpp VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Look for OpenCV
find_package(OpenCV REQUIRED)

# Check for Rockchip MPP library
find_library(MPP_LIB mpp)
if (NOT MPP_LIB)
    message(STATUS "Rockchip MPP library not found. Fetching and installing...")

    # Define the commands to fetch, build, and install MPP
    set(MPP_GIT_REPO "https://github.com/rockchip-linux/mpp.git")
    set(MPP_BUILD_COMMANDS
        COMMAND git clone ${MPP_GIT_REPO}
        COMMAND cd mpp && cmake -DRKPLATFORM=ON -DHAVE_DRM=ON && make
        COMMAND sudo make install
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    # Add custom target to fetch, build, and install MPP
    add_custom_target(
        BuildAndInstallMPP
        ${MPP_BUILD_COMMANDS}
    )

    # Ensure custom target is built before the main target
    add_dependencies(${PROJECT_NAME} BuildAndInstallMPP)
endif()

# Add the executable
add_executable(${PROJECT_NAME} cv_mpp.cpp)

# Link the required libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS} ${MPP_LIB})

# Include the required directories
target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})
