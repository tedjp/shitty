#include <iostream>

#include "Request.h"
#include "RequestHandler.h"
#include "Server.h"

using namespace shitty;

static void printRequest(Request&& request, Responder&& responder) {
    using std::cout;

    cout << request.method() << ' ' << request.path() << '\n';
    for (const auto& header: request.headers().kv_)
        cout << header.first << ": " << header.second << '\n';
    cout << '\n';

    if (!request.body().empty()) {
        cout << request.body();
        cout << '\n';
    }

    cout.flush();

    static const Response emptyResponse;
    responder.respond(emptyResponse);
}

int main(void) {
    try {
        Server server;
        server.addRoute("/", printRequest);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
