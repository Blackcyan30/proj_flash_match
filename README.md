# Flashmatch Core

Flashmatch is a small matching engine prototype. It is intended as the core of a
"Flashmatch" matching engine used for experimenting with matching
algorithms.

## Prerequisites

- GCC or Clang with C++23 support
- CMake 3.20 or newer

## Building with CMake

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

This produces the `flashmatch` executable in the `build/` directory.

## Running the example

```bash
./build/flashmatch
```

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
