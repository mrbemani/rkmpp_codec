# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(tsrkmpp VERSION 1.0)

# add include path
include_directories(./usr/include)


#set(CMAKE_SOURCE_DIR .)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include the toolchain file
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/arm-toolchain.cmake")

# Add the executable
add_executable(tsrkmpp main.cc)
