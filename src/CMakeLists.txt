add_subdirectory(util)
add_subdirectory(fs)
add_subdirectory(storage)

add_library(boltdb INTERFACE)
target_link_libraries(boltdb INTERFACE util fs storage)
