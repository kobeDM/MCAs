cmake_minimum_required(VERSION 2.8.8)
project (K102)

# Files required to build
set (SOURCE_FILES K102.c)

# Enable pthread library 
set(CMAKE_C_FLAGS -pthread)

include_directories(../KromekDriver/include ../SpectrometerDriver)
add_executable(K102 ${SOURCE_FILES})

target_link_libraries(K102 SpectrometerDriver)
