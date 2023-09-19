add_library(page freelist.cpp page.cpp)
AddClangTidy(page)
target_link_libraries(page PRIVATE util)