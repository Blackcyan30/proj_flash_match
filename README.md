# Flashmatch Core

Flashmatch is a simple prototype matching engine used to experiment with order matching algorithms in C++.

This project targets the C++23 standard.

## Manual Protobuf/gRPC Generation (Only if needed)

CMake automatically handles protobuf/gRPC code generation, but if you need to manually generate the files:

```bash
# From the project root directory
protoc -I=proto --cpp_out=proto --grpc_out=proto --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) proto/order_gateway.proto
```

This will generate the following files in the proto/ directory:
- order_gateway.pb.h
- order_gateway.pb.cc
- order_gateway.grpc.pb.h
- order_gateway.grpc.pb.cc

## Prerequisites

- GCC or Clang with C++20 support
- CMake 3.20 or newer
- gRPC and Protobuf

### Installing Dependencies

#### macOS
```bash
# Install required dependencies via Homebrew
brew install cmake grpc protobuf

# Verify installation
which protoc
which grpc_cpp_plugin
```

#### Ubuntu/Debian
```bash
# Install required dependencies
sudo apt update
sudo apt install -y build-essential cmake libgrpc++-dev libprotobuf-dev protobuf-compiler protobuf-compiler-grpc

# Verify installation
which protoc
which grpc_cpp_plugin
```

## Building and Running

### Option 1: Using Docker (Recommended)

The Docker approach ensures consistent builds across all environments, with specific versions:

- gRPC version 1.74.0 (built from source)
- C++20 standard
- Latest compatible Protobuf version

```bash
# Build the Docker image and start the container
docker-compose build
docker-compose up -d

# Enter the container shell
docker-compose exec flashmatch bash

# Inside the container, build the project manually using standard CMake commands
mkdir -p build
cd build
cmake ..
make -j$(nproc)

# Now you can run your executables:
./flashmatch
./orderbook_bench
```

#### For M1/M2 Macs (Apple Silicon):
```bash
# Set ARCH environment variable for Apple Silicon
ARCH=aarch64 docker-compose build
ARCH=aarch64 docker-compose up -d
```

#### To specify a different gRPC version:
```bash
# For x86_64 systems
GRPC_VERSION=v1.60.0 docker-compose up -d

# For M1/M2 Macs
GRPC_VERSION=v1.60.0 ARCH=aarch64 docker-compose up -d
```

#### C++20 Compatibility Note:
The Docker environment has been specifically configured to handle C++20 deprecation warnings in gRPC and Protobuf. In particular, C++20 deprecated certain implicit string literal to `const char*` conversions that were used in older versions of Protobuf-generated code. We use version v1.74.0 with specific compiler flags (`-Wno-deprecated-declarations`) to ensure compatibility. This specific version is pinned to guarantee reproducible builds even as newer versions are released.

#### Container Management:
```bash
# Stop the container when not in use
docker-compose down

# If you've created a container with docker run, you can:
docker start -ai flashmatch  # To restart and attach to an existing container
docker rm flashmatch         # To remove the container
```

### Option 2: Native Build

If you prefer to build natively after installing the required dependencies:

```bash
# Create build directory
mkdir -p build
cd build

# Configure
cmake ..

# Build all targets
make

# Or build specific targets
make flashmatch flashmatch_lib orderbook_bench
```

This creates the `flashmatch` and `orderbook_bench` executables in the `build/` directory.

## Running

From the repository root, run:

```bash
./build/flashmatch
```

## Progress

Days 1–4 complete. To benchmark, build and run `orderbook_bench` against a dataset:

```bash
cmake --build . --target orderbook_bench
./orderbook_bench <dataset>
```


The executable reads the dataset, warms up the matching engine, and reports
latency statistics (mean, median, p95, p99, and worst-case) for the processed
orders.

Example output:

```bash
$ ./orderbook_bench ../datasets/sample.csv
Initializing Benchmark !
Warming up the matching engine with 100 orders...
Starting benchmarking with 900 orders...
=== Flashmatch Benchmark ===
Total orders:          1000
Warmup orders:         100
Orders processed:      900
Mean latency:          3.2 micro-seconds
Median latency:        3.1 micro-seconds
95th percentile:       4.5 micro-seconds
99th percentile:       5.7 micro-seconds
Worst-case latency:    9.8 micro-seconds
CPU:                   processor  : 0
Compiler flags:        -O3 -march=native
Benchmark completed.
```


## License

Flashmatch is licensed under the [MIT](LICENSE) License.

## Docker Usage Details

### Using docker-compose (Recommended)

We've included a `docker-compose.yml` file for easier container management:

```bash
# Start the container in background
docker-compose up -d

# Enter the container shell
docker-compose exec flashmatch bash

# Exit the container shell (but leave container running)
exit

# Stop and remove the container
docker-compose down
```

### Manual Docker Commands

If you prefer using raw Docker commands:

```bash
# Build the image
docker build --build-arg ARCH=x86_64 -t flashmatch:1 .

# Run a new container
docker run -it \
  --name flashmatch \
  -v "$(pwd)":/flashmatch \
  -w /flashmatch \
  -v /run/host-services/ssh-auth.sock:/ssh-agent \
  -e SSH_AUTH_SOCK=/ssh-agent \
  flashmatch:1

# Start an already built container
docker start -ai flashmatch

# Exit the container (container will stop)
exit
```

### For ARM64/Apple Silicon:
```bash
docker build --build-arg ARCH=aarch64 -t flashmatch:1 .
```

exit

## To explicitly stop a container:

docker stop flashmatch



Install protobuf,
Install grpc