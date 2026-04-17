cmake_minimum_required(VERSION 2.6)
project (SpectrometerDriver)

find_path (LIBUDEV_H_PATH "libudev.h")
find_library (LIBUDEV_LIB_PATH udev)

set (SOURCE_FILES Detector.cpp DriverMgr.cpp SpectrometerDriver.cpp stdafx.cpp)

include_directories(${LIBUDEV_H_PATH})
include_directories(. ../KromekDriver/include)

# Required for use in shared lib
set(CMAKE_CXX_FLAGS "-fPIC -Wno-enum-compare")

# Create both a static and shared lib
add_library (SpectrometerDriverStatic STATIC ${SOURCE_FILES})
add_library (SpectrometerDriver SHARED ${SOURCE_FILES})

target_link_libraries(SpectrometerDriverStatic KromekDriver ${LIBUDEV_LIB_PATH})
target_link_libraries(SpectrometerDriver KromekDriver ${LIBUDEV_LIB_PATH})
add_definitions (-DUSBSPECTROMETERDLL_EXPORTS -D_UNICODE)
