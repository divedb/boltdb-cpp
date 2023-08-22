# file(GLOB SOURCES *.cpp)
# add_library(db ${SOURCES})
add_library(db db.cpp)
AddClangTidy(db)