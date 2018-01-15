cmake_minimum_required(VERSION 2.8)

project(joycon)

set(CMAKE_CXX_STANDARD_REQUIRED 14)
set(CMAKE_CXX_STANDARD 14)



set(SOURCES
    main.cpp
    buffer.cpp
    joycon.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} hidapi-hidraw pthread)

# for coveralls in travis
set(COVERAGE OFF CACHE BOOL "Coverage")

# enable tests per default
option(BUILD_TESTS "Build automatic tests" ON)

if (BUILD_TESTS)
	#tests/googletest contains CMakeLists.txt to download and compile googletests
	add_subdirectory(tests/googletest)
	enable_testing() #tell cmake we are using a testing framework

	#tests/ includes every testcase that should be build
	add_subdirectory(tests)
endif()

