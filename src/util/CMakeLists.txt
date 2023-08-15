file(GLOB SOURCES *.cpp)
add_library(util ${SOURCES})
AddClangTidy(util)