add_subdirectory(db)
add_subdirectory(fs)
add_subdirectory(os)
# add_subdirectory(storage)
add_subdirectory(util)
add_subdirectory(page)

add_library(boltdb INTERFACE)
target_link_libraries(boltdb INTERFACE db fs os util page)
