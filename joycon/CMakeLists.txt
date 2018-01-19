cmake_minimum_required(VERSION 2.8)

project(joycon)

set(CMAKE_CXX_STANDARD_REQUIRED 14)
set(CMAKE_CXX_STANDARD 14)

set(SOURCES
    buffer.cpp
    joycon.cpp
	rumble.cpp
	homelight.cpp)

add_executable(${PROJECT_NAME} main.cpp ${SOURCES})
target_link_libraries(${PROJECT_NAME} hidapi-hidraw pthread)

add_executable(${PROJECT_NAME} music.cpp ${SOURCES})
target_link_libraries(${PROJECT_NAME} hidapi-hidraw pthread)

# enable tests per default
option(BUILD_TESTS "Build automatic tests" ON)

if (BUILD_TESTS)
	#tests/googletest contains CMakeLists.txt to download and compile googletests
	add_subdirectory(tests/googletest)
	enable_testing() #tell cmake we are using a testing framework

	#tests/ includes every testcase that should be build
	add_subdirectory(tests)
endif()

