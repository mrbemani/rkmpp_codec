# Set the path to the toolchain installation
set(TOOLCHAIN_PATH "/home/shiqi/opt/FriendlyARM/toolschain/4.4.3")

# Specify the cross compiler
set(CMAKE_C_COMPILER "${TOOLCHAIN_PATH}/arm-none-linux-gnueabi/bin/arm-linux-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PATH}/arm-none-linux-gnueabi/bin/arm-linux-g++")

# Specify additional compiler and linker flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --sysroot=${TOOLCHAIN_PATH}/arm-none-linux-gnueabi/sys-root")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --sysroot=${TOOLCHAIN_PATH}/arm-none-linux-gnueabi/sys-root")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --sysroot=${TOOLCHAIN_PATH}/arm-none-linux-gnueabi/sys-root")

# You may need to set additional variables, such as the target architecture, etc.
