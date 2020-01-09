# shitty — A C++ HTTP Server library

## Examples

### [HelloWorld](HelloWorld.cpp)

A simple example of serving static responses.

### [ProxyServer](ProxyServer.cpp)

A simple HTTP proxy.

### [PrintRequestServer](PrintRequestServer.cpp)

Prints incoming requests.

## Usage

    make && sudo ./hello-world

## HPACK — HTTP/2 header compression

Includes a functional implementation of HPACK, HTTP/2's header
compression/decompression algorithm. It's not integrated into the server just
yet.
See the `hpack` & `hpack/huffy` directories.
