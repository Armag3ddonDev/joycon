include_directories(../../) #to include all *.cpp and *.h files

add_executable(outputbuffer main.cpp buffer.cpp)
target_link_libraries(outputbuffer gtest_main gmock_main)
add_test(NAME testoutputbuffer COMMAND outputbuffer)

