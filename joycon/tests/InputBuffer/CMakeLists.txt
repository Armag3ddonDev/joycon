include_directories(../../) #to include all *.cpp and *.h files

add_executable(inputbuffer main.cpp ../../buffer.cpp)
target_link_libraries(inputbuffer gtest_main gmock_main)
add_test(NAME testinputbuffer COMMAND inputbuffer)
