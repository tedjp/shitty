#include "tcpserver.h"

class WTFServer: public shitty::tcpserver::Server {
public:
    WTFServer():
        Server(23206)
    {}

    void onNewConnection(shitty::Socket&& client) override {
        const char response[] = {
            'w', 't', 'f', '\n'
        };

        client.send(response, sizeof(response));

        client.close();
    }
};

int main(void) {
    WTFServer().run();
    return 0;
}
