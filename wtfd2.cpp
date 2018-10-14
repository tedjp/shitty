#include "tcpserver.h"

class WTFHandler: public shitty::TCPServer::Handler {
public:
    void onAccept(shitty::Socket&& socket) override {
        const char response[] = {
            'w', 't', 'f', '\n'
        };

        socket.send(response, sizeof(response));

        socket.close();
    }
};

int main(void) {
    WTFHandler handler;
    auto server = shitty::makeTCPServer(handler, 23206);
    server.run();
    return 0;
}
