#include <unistd.h>

#include "SignalReceiver.h"

#include "Error.h"

namespace shitty {

SignalReceiver::SignalReceiver(int signum, std::function<void(int)> action):
    source_(signum),
    action_(move(action))
{}

int SignalReceiver::getPollFD() const {
    return source_.getPollFD();
}

void SignalReceiver::onPollIn() {
    int signum = -1;
    ssize_t len = read(getPollFD(), &signum, sizeof(signum));
    if (len == -1)
        throw error_errno("read");

    if (len != sizeof(signum))
        throw std::runtime_error("poll socket closed");

    action_(signum);
}

} // namespace shitty
