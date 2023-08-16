add_library(storage page.cpp)
AddClangTidy(storage)
target_link_libraries(storage PRIVATE util)