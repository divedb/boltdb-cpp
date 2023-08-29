add_executable(meta_test meta_test.cpp)
target_link_libraries(meta_test PRIVATE gtest boltdb)

add_executable(db_test db_test.cpp)
target_link_libraries(db_test PRIVATE gtest boltdb)