add_executable(page_test page_test.cpp)
target_link_libraries(page_test PRIVATE boltdb gtest)

add_executable(freelist_test freelist_test.cpp)
target_link_libraries(freelist_test PRIVATE boltdb gtest)
