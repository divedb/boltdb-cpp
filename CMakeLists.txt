cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(boltdb CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMKAE_CXX_FLAGS} -g -Wall")
set(BOLTDB_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(GOOGLE_TEST_INCLUDE_DIR ${PROJECT_BINARY_DIR}/_deps/googletest-src/googletest/include)

include_directories(${BOLTDB_INCLUDE_DIR})
include_directories(${GOOGLE_TEST_INCLUDE_DIR})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(ClangTidy)

# Include googletest
Include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    # release-1.11.0
    GIT_TAG        e2239ee6043f73722e7aa812a459f54a28552929
)
FetchContent_MakeAvailable(googletest)

add_subdirectory(src)
add_subdirectory(tests)