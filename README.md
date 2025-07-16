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

## License

Flashmatch is licensed under the [MIT](LICENSE) License.

## To Build image:

The Dockerfile supports building for ARM64 (default) or x86_64. To specify an
architecture, pass the `ARCH` build argument. For example, to build for x86_64:

```bash
docker build --build-arg ARCH=x86_64 -t flashmatch:1 .
```
If no architecture is supplied, the image is built for ARM64.

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
