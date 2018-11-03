#include <unistd.h>

#include "server.h"

namespace shitty {

class Server::Impl {
public:
    Impl(Server *server):
        server_(server)
    {}

    void run();

    ~Impl() {
        close(epfd_);
    }

private:
    Server *server_ = nullptr;
    int epfd_ = -1;
};

Server::Server():
    impl_(std::make_unique<Impl>(this))
{}

void Server::run() {
    impl_->run();
}

void Server::Impl::run() {
    // TODO
}

Server::~Server() {
}

} // namespace
