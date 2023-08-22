add_executable(options_test options_test.cpp)
target_link_libraries(options_test PRIVATE gtest)

add_executable(alloc_test alloc_test.cpp)
target_link_libraries(alloc_test PRIVATE boltdb gtest)

add_executable(slice_test slice_test.cpp)
target_link_libraries(slice_test PRIVATE boltdb gtest)

add_executable(ref_count_test ref_count_test.cpp)
target_link_libraries(ref_count_test PRIVATE boltdb gtest)

add_executable(format_test format_test.cpp)
target_link_libraries(format_test PRIVATE gtest)

add_executable(crc64_test crc64_test.cpp)
target_link_libraries(crc64_test PRIVATE boltdb gtest)

add_executable(binary_test binary_test.cpp)
target_link_libraries(binary_test PRIVATE boltdb gtest)