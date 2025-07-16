cmake_minimum_required(VERSION 3.20)
project(Flashmatch)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Library with core functionality
add_library(flashmatch_lib
  flashmatch.cpp
  lock_free_queue.cpp
)
target_include_directories(flashmatch_lib PUBLIC ${PROJECT_SOURCE_DIR})
target_compile_options(flashmatch_lib PRIVATE -O3 -march=native)

add_executable(flashmatch main.cpp)
target_link_libraries(flashmatch PRIVATE flashmatch_lib)

# GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# Prevent GoogleTest from overriding our compiler/linker options
set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_subdirectory(tests)
