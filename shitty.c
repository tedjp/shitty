#define _GNU_SOURCE 1
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>

static const int TCP_BACKLOG = 1;

enum FrameType {
    GOAWAY = 0x07
};

enum ErrorCode {
    NO_ERROR = 0x00,
    PROTOCOL_ERROR = 0x01
};

// length doesn't include the length of the frame header
static ssize_t write_frame_header(int fd, uint32_t length, uint8_t type, uint8_t flags, uint32_t stream_id) {
    if (stream_id & (1u << 31)) {
        errno = EINVAL;
        return -1;
    }

    struct __attribute__((packed)) frame_header {
        uint32_t nbo_length_type; // u24(length), u8(type)
        uint8_t flags;
        uint32_t nbo_stream_id; // actually u1(r) + u31(id)
    } frame_header;

    if (length > (1 << 23)) {
        errno = EINVAL;
        return -1;
    }

    frame_header.nbo_length_type = htonl(length << 8 | type);
    frame_header.flags = flags;
    frame_header.nbo_stream_id = htonl(stream_id);

    return write(fd, &frame_header, sizeof(frame_header));
}

static ssize_t write_frame(
        int sock,
        uint32_t stream_id,
        enum FrameType type,
        uint8_t flags,
        void* frame_data,
        size_t len)
{
    ssize_t r = write_frame_header(sock, len, type, flags, stream_id);
    if (r == -1)
        return -1;

    return write(sock, frame_data, len);
}

static ssize_t write_goaway_frame(
        int sock,
        uint32_t last_stream_id,
        uint32_t error_code)
{
    struct __attribute__((packed)) goaway_frame {
        uint32_t nbo_r_last_stream_id;
        uint32_t nbo_error_code;
    } frame;

    if (last_stream_id & (1u << 31)) {
        errno = EINVAL;
        return -1;
    }

    frame.nbo_r_last_stream_id = htonl(last_stream_id);
    frame.nbo_error_code = htonl((uint32_t)error_code);

    return write_frame(
            sock,
            0u, // stream ID is always 0 for GOAWAY
            GOAWAY,
            0x00, // GOAWAY has no flags defined
            &frame,
            sizeof(frame));
}

// Talk to a client once we've determined that it wants to UPGRADE!!!
static void http2(int client) {
    // We're now talking in HTTP/2 frames
    //
    // Naturally we want to tell the client to GOAWAY.
    // NO_ERROR indicates clean shutdown.
    ssize_t err = write_goaway_frame(client, 0u, NO_ERROR);
    if (err == -1)
        perror("Failed to write GOAWAY");
}

static int upgrade(int client) {
    char response[] = "HTTP/1.1 101 Switching Protocols\r\n"
                      "Connection: Upgrade\r\n"
                      "Upgrade: h2c\r\n"
                      "\r\n";
    if (write(client, response, sizeof(response) - 1) == -1) {
        fprintf(stderr, "Failed to write upgrade message to client: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

static void handle(int client) {
    // This is where something like Wangle would be useful
    // The parsing of HTTP/1.1 messages is totally bad, but
    // passable for sane clients.

    char reqbuf[4096]; // bleh

    ssize_t len = read(client, reqbuf, sizeof(reqbuf));
    // Obviously we should be looping until \r\n\r\n is found or some timeout
    fprintf(stderr, "DBG: Read %zd octets\n", len);
    if (len < sizeof(reqbuf))
        reqbuf[len] = '\0';
    else
        reqbuf[sizeof(reqbuf)-1] = '\0'; // yep actually truncating the request. This is garbage.

    if (strcasestr(reqbuf, "\r\nUpgrade: h2c\r\n") == NULL) {
        // Client didn't attempt to upgrade. Drop it for being old and lame.
        static char body[] = "Upgrade required.\n"; // IF CHANGED UPDATE Content-Length
        static char headers[] = "HTTP/1.1 426 Upgrade Required\r\n"
                                // Upgrade response header is mandatory for 426
                                "Upgrade: h2c\r\n"
                                "Content-Type: text/plain; charset=us-ascii\r\n"
                                // Don't really care if content headers are wrong;
                                // we're abusing clients after all.
                                "Content-Length: 18\r\n"
                                "Connection: close, Upgrade\r\n"
                                "\r\n";
        // Don't really care if writes fail; just gonna close
        // the connection and abandon the client anyway.
        // We'll have a modicum of decency and only write the body if the
        // headers were probably sent.
        if (write(client, headers, sizeof(headers) - 1) != -1)
            write(client, body, sizeof(body) - 1);

        if (close(client) != 0)
            perror("Failed to close client socket");

        return;
    }

    if (upgrade(client) != 0) {
        fprintf(stderr, "Failed to upgrade to HTTP/2\n");
        return;
    }

    // TODO: Pass HTTP2-Settings request-header
    http2(client);
}

static void run(int s) {
    int client = -1;

    while ((client = accept(s, NULL, NULL)) != -1) {
        handle(client);
    }
}

int main(void) {
    struct addrinfo hints;
    struct addrinfo *addrs = NULL;
    memset(&hints, '\0', sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_V4MAPPED | AI_ADDRCONFIG;
    // This root-detection hack is garbage in the day of capabilities, of course
    int err = getaddrinfo(NULL, getuid() == 0 ? "http" : "http-alt", &hints, &addrs);

    if (err != 0) {
        fprintf(stderr, "Failed to get bind address: %s\n", gai_strerror(err));
        return 1;
    }

    int s = -1;

    for (struct addrinfo *addr = addrs; s == -1 && addr != NULL; addr = addr->ai_next) {
        s = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (s == -1) {
            fprintf(stderr, "Failed to create socket: %s\n", strerror(errno));
            continue;
        }

        err = bind(s, addr->ai_addr, addr->ai_addrlen);
        if (err) {
            fprintf(stderr, "Failed to bind: %s\n", strerror(errno));
            close(s);
            s = -1;
            continue;
        }

        err = listen(s, TCP_BACKLOG);
        if (err) {
            fprintf(stderr, "Failed to listen: %s\n", strerror(errno));
            close(s);
            s = -1;
            continue;
        }
    }

    freeaddrinfo(addrs);
    addrs = NULL;

    if (s == -1) {
        fprintf(stderr, "Failed to bind a working socket; giving up.\n");
        return 1;
    }

    run(s);

    return 1;
}
