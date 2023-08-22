file(GLOB SOURCES *.cpp)
add_library(os ${SOURCES})
AddClangTidy(os)