add_library(file_system file_system.cpp)
AddClangTidy(file_system)
target_link_libraries(file_system PRIVATE util)
