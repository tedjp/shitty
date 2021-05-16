# shitty — A C++ HTTP Server

## Examples

### [HelloWorld](HelloWorld.cpp)

A simple example of serving static responses.

### [ProxyServer](ProxyServer.cpp)

A simple HTTP proxy.

### [PrintRequestServer](PrintRequestServer.cpp)

Prints incoming requests.

## Dependencies

### C++20

A C++20-capable compiler is required. I suggest
[GCC 11](https://gcc.gnu.org/gcc-11/) or later.

As of early 2021, you might need to install a newer compiler than you
already have, for example from
[Debian experimental](https://packages.debian.org/experimental/g++-11) or
[Ubuntu
toolchain-r](https://launchpad.net/~ubuntu-toolchain-r/+archive/ubuntu/test).

### Included dependencies

[**fb64**](https://github.com/tedjp/fb64) is required for HTTP/2 upgrade.

Clone submodules to ensure it's available:

    git submodule init
    git submodule update

## Usage

    make depend && make -j && sudo ./hello-world

## HPACK — HTTP/2 header compression

Includes a functional implementation of HPACK, HTTP/2's header
compression/decompression algorithm. Integration and implementation of HTTP/2 is
underway.

See [dependencies/hpack](dependencies/hpack) for the HPACK implementation.
