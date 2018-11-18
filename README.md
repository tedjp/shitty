# shitty — A C++ HTTP Server library

## Examples

### [HelloWorld](HelloWorld.cpp)

A simple example of serving static responses.

### [CountingServer](CountingServer.cpp)

A server that maintains some state, responding to each request with the number
of requests served so far.

### [PerRequestServer](PerRequestServer.cpp)

An example of a server that creates a new handler instance for each request.

## Usage

    make && sudo ./hello-world

## HPACK — HTTP/2 header compression

Includes a functional implementation of HPACK, HTTP/2's header
compression/decompression algorithm. It's not integrated into the server just
yet.
See the `hpack` & `hpack/huffy` directories.
