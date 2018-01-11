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
