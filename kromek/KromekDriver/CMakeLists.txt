cmake_minimum_required(VERSION 2.6)
project (KromekDriver)

find_path (LIBUDEV_H_PATH "libudev.h")
find_library (UDEV_LIB_PATH udev)
find_library (RT_LIB_PATH rt)

set (SOURCE_FILES src/CriticalSection.cpp src/D3DataProcessor.cpp src/DeviceBase.cpp src/DeviceMgr.cpp src/Event.cpp src/GR1.cpp src/IntervalCountProcessor.cpp src/K102.cpp src/Lock.cpp src/RadAngel.cpp src/RollingQueue.cpp src/SIGMA_25.cpp src/SIGMA_50.cpp src/stdafx.cpp src/Thread.cpp src/TN15.cpp src/GR05.cpp src/DeviceEnumeratorLinux.cpp src/USBKromekDataInterfaceLinux.cpp src/UNIBASE.cpp )

#set (HEADER_FILES include/CriticalSection.h include/D3DataProcessor.h include/D3Structs.h include/DeviceBase.h include/DeviceMgr.h include/Event.h include/GR1.h include/IDataInterface.h include/IDataProcessor.h include/IDevice.h include/IntervalCountProcessor.h include/K102.h include/kromek.h include/Lock.h include/RadAngel.h  include/RollingQueue.h include/SIGMA_25.h include/SIGMA_50.h include/stdafx.h include/targetver.h include/Thread.h include/Time.h include/TN15.h include/GR05.h include/types.h include/version.h include/kromek_endian.h include/UNIBASE.h)

include_directories(${LIBUDEV_H_PATH})
include_directories(include)

# Required for use in shared lib
set(CMAKE_CXX_FLAGS -fPIC)

add_library (KromekDriver STATIC ${SOURCE_FILES})
add_definitions (-DKROMEKDRIVER_EXPORTS -D_UNICODE)
target_link_libraries(KromekDriver ${UDEV_LIB_PATH} ${RT_LIB_PATH})
