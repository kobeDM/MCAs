cmake_minimum_required (VERSION 2.6)

# Default to release build type
if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE RELEASE)
endif()

# Set output directorys for shared and static libs
if (CMAKE_SIZEOF_VOID_P EQUAL 4)
	set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/libs/linux32/release")
	set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/libs/linux32/release")
	set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/libs/linux32/release")
	set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/libs/linux32/release")	
	set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/libs/linux32/debug")
	set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/libs/linux32/debug")
else()
	set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/libs/linux64/release")
	set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/libs/linux64/release")
	set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/libs/linux64/release")
	set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/libs/linux64/release")
	set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/libs/linux64/debug")
	set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/libs/linux64/debug")
endif()

add_subdirectory (KromekDriver)
add_subdirectory (SpectrometerDriver)
add_subdirectory (K102)
#add_subdirectory (ConsoleExample)
#add_subdirectory (LanternBeacon)

# QT is required for the example. Add an option to disable it
#option (BUILD_EXAMPLE "Build example app - requires QT" ON)
option (BUILD_EXAMPLE "Build example app - requires QT" OFF)

if (BUILD_EXAMPLE)
  add_subdirectory (SpectrometerExample)
  add_subdirectory (LanternBeacon)
endif (BUILD_EXAMPLE)
