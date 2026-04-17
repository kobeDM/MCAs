cmake_minimum_required(VERSION 2.8.8)
project (ConsoleExample)

# Files required to build
set (SOURCE_FILES console.c)

# Enable pthread library 
set(CMAKE_C_FLAGS -pthread)

include_directories(../KromekDriver/include ../SpectrometerDriver)
add_executable(ConsoleExample ${SOURCE_FILES})

target_link_libraries(ConsoleExample SpectrometerDriver)
