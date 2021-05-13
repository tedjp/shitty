# shitty — A C++ HTTP Server library

## Examples

### [HelloWorld](HelloWorld.cpp)

A simple example of serving static responses.

### [ProxyServer](ProxyServer.cpp)

A simple HTTP proxy.

### [PrintRequestServer](PrintRequestServer.cpp)

Prints incoming requests.

## Dependencies

[**fb64**](https://github.com/tedjp/fb64) is required for HTTP/2 upgrade.

Clone submodules to ensure it is available:

    git submodule init
    git submodule update

## Usage

    make depend && make && sudo ./hello-world

## HPACK — HTTP/2 header compression

Includes a functional implementation of HPACK, HTTP/2's header
compression/decompression algorithm. It's not integrated into the server just
yet.
See the `hpack` & `hpack/huffy` directories.
