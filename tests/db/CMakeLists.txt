add_executable(meta_test meta_test.cpp)
target_link_libraries(meta_test PRIVATE gtest boltdb)