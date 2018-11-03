#include <sys/epoll.h>

#include "error.h"
#include "server.h"

namespace shitty::http2 {

Server::Server():
    tcpserver::Server()
{
    epollfd_ = SafeFD(epoll_create1(EPOLL_CLOEXEC));
    if (!epollfd_)
        throw error_errno("epoll_create1");
}

void Server::onNewConnection(Socket&& client) {
    int fd = client.getRawFD();

    auto inserted = clients_.insert(std::make_pair(std::move(client), ClientHandler()));

    struct epoll_event event = {
        .events = EPOLLIN,
        .data = {
            .ptr = &inserted.first->second,
        },
    };

    if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &event) == -1) {
        int errno_save = errno;
        clients_.erase(inserted.first);
        client.close();
        errno = errno_save;
        throw error_errno("epoll_ctl");
    }
}

void Server::run() {
    //epoll
}

} // namespace shitty::http2
