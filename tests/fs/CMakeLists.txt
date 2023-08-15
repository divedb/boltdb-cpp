add_executable(file_system_test file_system_test.cpp)
target_link_libraries(file_system_test PRIVATE fs gtest)

add_executable(file_handle_test file_handle_test.cpp)
target_link_libraries(file_handle_test PRIVATE fs gtest)
