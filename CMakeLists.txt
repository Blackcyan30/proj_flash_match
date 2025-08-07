cmake_minimum_required(VERSION 3.20)
project(Flashmatch)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)


# lock_free_queue INTERFACE lib
add_library(lock_free_queue INTERFACE)
target_include_directories(lock_free_queue INTERFACE ${PROJECT_SOURCE_DIR}/include)

# gRPC / Protobuf
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# Library with core functionality
add_library(flashmatch_lib
  src/flashmatch.cpp
  src/order_book.cpp
  src/matching_engine.cpp
)

target_include_directories(flashmatch_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_options(flashmatch_lib PRIVATE -O3 -march=native)
target_link_libraries(flashmatch_lib PRIVATE lock_free_queue)

add_executable(flashmatch src/main.cpp)
target_link_libraries(flashmatch PRIVATE flashmatch_lib)

# Benchmark executable
add_executable(orderbook_bench src/benchmark.cpp)
target_link_libraries(orderbook_bench PRIVATE flashmatch_lib)
target_compile_options(orderbook_bench PRIVATE -O3 -march=native)

# Proto generated sources
set(PROTO_SRC
  proto/order_gateway.pb.cc
  proto/order_gateway.grpc.pb.cc
)

add_library(order_gateway_proto ${PROTO_SRC})
target_include_directories(order_gateway_proto PUBLIC ${PROJECT_SOURCE_DIR}/proto)
target_link_libraries(order_gateway_proto PUBLIC protobuf::libprotobuf gRPC::grpc++)

add_executable(order_gateway_server src/order_gateway_server.cpp)
target_link_libraries(order_gateway_server PRIVATE order_gateway_proto lock_free_queue gRPC::grpc++)

add_executable(order_gateway_client src/order_gateway_client.cpp)
target_link_libraries(order_gateway_client PRIVATE order_gateway_proto gRPC::grpc++)

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
include(GoogleTest)

add_subdirectory(tests)
