# Flashmatch Core

Flashmatch is a simple prototype matching engine used to experiment with order matching algorithms in C++.

This project targets the C++23 standard.

## Prerequisites

- GCC or Clang with C++23 support
- CMake 3.20 or newer

## Building

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

This creates the `flashmatch` executable in the `build/` directory.

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

## To Build image:

Choose the desired architecture when building the Docker image. For x86_64 run:

```bash
docker build --build-arg ARCH=x86_64 -t flashmatch:1 .
```

For ARM64 run:

```bash
docker build --build-arg ARCH=aarch64 -t flashmatch:1 .
```

## To build & run container:

docker run -it \
 --name flashmatch \
 -v "$(pwd)":/flashmatch \
 -w /flashmatch \
 -v /run/host-services/ssh-auth.sock:/ssh-agent \
 -e SSH_AUTH_SOCK=/ssh-agent \
 flashmatch:1

## To start an already built container:

docker start -ai flashmatch

## To exit when in a container:

exit

## To explicitly stop a container:

docker stop flashmatch
