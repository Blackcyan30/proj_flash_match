cmake_minimum_required(VERSION 3.20)
project(Flashmatch LANGUAGES CXX)

# ---- Language / global opts ---------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ---- Dependencies ------------------------------------------------------------
# Find packages installed on the system
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# ---- Project libraries / includes --------------------------------------------
add_library(lock_free_queue INTERFACE)
target_include_directories(lock_free_queue INTERFACE ${PROJECT_SOURCE_DIR}/include)

# ---- Protobuf / gRPC Code Generation ---------------------------------------------
# This section defines how to generate C++ code from .proto files

# Define paths for our .proto files and generated code
set(PROTO_DIR ${PROJECT_SOURCE_DIR}/proto)                  # Where .proto files live
set(PROTO_FILE ${PROTO_DIR}/order_gateway.proto)            # Our proto file
set(GENERATED_DIR ${PROTO_DIR})                             # Output generated code directly to proto folder

# Find the protoc compiler and grpc plugin
find_program(PROTOC protoc)
find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)

# Add debug messages to see if protoc and plugin are found
message(STATUS "PROTOC found: ${PROTOC}")
message(STATUS "GRPC_CPP_PLUGIN found: ${GRPC_CPP_PLUGIN}")
message(STATUS "Proto file path: ${PROTO_FILE}")
message(STATUS "Generated directory: ${GENERATED_DIR}")

# Check if the proto file exists
if(NOT EXISTS ${PROTO_FILE})
  message(FATAL_ERROR "Proto file not found: ${PROTO_FILE}")
endif()

# List the files that will be generated from our .proto file
set(GENERATED_SRCS
  ${GENERATED_DIR}/order_gateway.pb.cc                      # Protobuf message code
  ${GENERATED_DIR}/order_gateway.grpc.pb.cc                 # gRPC service code
)
set(GENERATED_HDRS
  ${GENERATED_DIR}/order_gateway.pb.h                       # Protobuf message headers
  ${GENERATED_DIR}/order_gateway.grpc.pb.h                  # gRPC service headers
)

# Check if the generated files already exist
set(NEEDS_GENERATION FALSE)
foreach(FILE ${GENERATED_SRCS} ${GENERATED_HDRS})
  if(NOT EXISTS ${FILE})
    set(NEEDS_GENERATION TRUE)
    break()
  endif()
endforeach()

message(STATUS "Checking for generated protobuf files...")
if(NEEDS_GENERATION)
  message(STATUS "  Missing generated files - will generate from ${PROTO_FILE}")
  
  # This command runs the protoc compiler to generate C++ code
  add_custom_command(
    OUTPUT ${GENERATED_SRCS} ${GENERATED_HDRS}                # Generated files
    COMMAND ${PROTOC}                                         # The protobuf compiler
    ARGS
      -I ${PROTO_DIR}                                         # Where to find .proto files
      --cpp_out=${GENERATED_DIR}                              # Generate regular protobuf code
      --grpc_out=${GENERATED_DIR}                             # Generate gRPC service code
      --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}             # Use the gRPC plugin
      ${PROTO_FILE}                                           # Input .proto file
    DEPENDS ${PROTO_FILE}                                     # Dependencies
    COMMENT "Generating C++ sources from ${PROTO_FILE}"
    VERBATIM
  )
  
  add_custom_target(generate_proto_cpp
    DEPENDS ${GENERATED_SRCS} ${GENERATED_HDRS}
  )
else()
  message(STATUS "  All generated protobuf files exist - skipping generation")
  add_custom_target(generate_proto_cpp)
endif()

# A tiny library to house generated files so linking is simple
add_library(order_gateway_proto ${GENERATED_SRCS})
add_dependencies(order_gateway_proto generate_proto_cpp)
target_include_directories(order_gateway_proto PUBLIC ${GENERATED_DIR})
target_link_libraries(order_gateway_proto PUBLIC protobuf::libprotobuf gRPC::grpc++)
set_property(TARGET order_gateway_proto PROPERTY CXX_STANDARD 20)

# ---- Your core library / executables -----------------------------------------
add_library(flashmatch_lib
  src/flashmatch.cpp
  src/order_book.cpp
  src/matching_engine.cpp
)
target_include_directories(flashmatch_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_options(flashmatch_lib PRIVATE -O3 -march=native)
target_link_libraries(flashmatch_lib PRIVATE lock_free_queue)
set_property(TARGET flashmatch_lib PROPERTY CXX_STANDARD 20)

add_executable(flashmatch src/main.cpp)
target_link_libraries(flashmatch PRIVATE flashmatch_lib)
set_property(TARGET flashmatch PROPERTY CXX_STANDARD 20)

add_executable(orderbook_bench src/benchmark.cpp)
target_link_libraries(orderbook_bench PRIVATE flashmatch_lib)
target_compile_options(orderbook_bench PRIVATE -O3 -march=native)
set_property(TARGET orderbook_bench PROPERTY CXX_STANDARD 20)

# The gRPC server and client executables are commented out because
# their source files are empty. Uncomment these when you implement them.

# add_executable(order_gateway_server src/order_gateway_server.cpp)
# target_link_libraries(order_gateway_server PRIVATE order_gateway_proto lock_free_queue gRPC::grpc++)
# set_property(TARGET order_gateway_server PROPERTY CXX_STANDARD 20)

# add_executable(order_gateway_client src/order_gateway_client.cpp)
# target_link_libraries(order_gateway_client PRIVATE order_gateway_proto gRPC::grpc++)
# set_property(TARGET order_gateway_client PROPERTY CXX_STANDARD 20)

# ---- Tests --------------------------------------------------------------------
include(CTest)
include(FetchContent)
if(BUILD_TESTING)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  )
  set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
  enable_testing()
  include(GoogleTest)
  add_subdirectory(tests)
endif()
