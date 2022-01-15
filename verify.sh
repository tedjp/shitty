#!/bin/sh
set -e
set -x

# These are intended to be run against the `print-requests` server, or
# equivalent.

curl -v --http1.1 http://localhost/
curl -v --http2 http://localhost/
curl -v --http2-prior-knowledge http://localhost/

# Short HTTP1 request
echo -en "GET / HTTP/1.1\r\nConnection: close\r\n\r\n" | nc -v localhost 80

echo OK
