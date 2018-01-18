include_directories(../../) #to include all *.cpp and *.h files

add_executable(outputbuffer main.cpp ../../buffer.cpp ../../rumble.cpp)
target_link_libraries(outputbuffer gtest_main gmock_main)
add_test(NAME testoutputbuffer COMMAND outputbuffer)

if (COVERAGE)
    target_compile_options(outputbuffer PRIVATE --coverage)
    target_link_libraries(outputbuffer PRIVATE --coverage)
endif()
