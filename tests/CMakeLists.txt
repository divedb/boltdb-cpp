add_executable(alloc_test alloc_test.cpp)
add_executable(slice_test slice_test.cpp)

set_target_properties(boltdb PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(slice_test boltdb)
