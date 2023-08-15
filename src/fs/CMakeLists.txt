add_library(fs file_system.cpp)
AddClangTidy(fs)
target_link_libraries(fs PRIVATE util)
