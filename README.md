# shitty - A useless HTTP/2 server
Upgrades HTTP/1 clients to HTTP/2, then tells
them to GOAWAY and closes the connection.

HTTP/1.1 clients get an "Upgrade Required" message.

It's single threaded which is either because I'm too lazy to make it
high-performance, event-driven *and* multi-threaded, or because being
single-threaded makes it more shitty.

Runs on port 8080, or port 80 if you like to live dangerously and run untrusted
code as root.

Usage
=====

    make run

See also
========

If you like useless daemons, you might also be interested in
[wtfd](http://tedp.id.au/junkcode/wtfd.c).
