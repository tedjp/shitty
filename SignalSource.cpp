#include <signal.h>
#include <unistd.h>

#include "Error.h"
#include "SignalSource.h"

using namespace shitty;

// A single fd & handler is used to meet the sighandler_t API for the signal() call
static int write_fd = -1;

static void handle(int sig) {
    write(write_fd, &sig, sizeof(sig));
}

static void set_fd(int fd) {
    if (write_fd != -1 && fd != -1)
        throw std::runtime_error("Only one SignalSource is supported right now");

    write_fd = fd;
}

static void clear_fd() {
    write_fd = -1;
}

SignalSource::SignalSource(int signum):
    signum_(signum)
{
    setupPipe();
    setupSignalHandler();
}

SignalSource::~SignalSource() {
    removeSignalHandler();
    closePipe();
}

int SignalSource::getPollFD() const {
    return read_pipe_;
}

void SignalSource::setupPipe() {
    int pipefd[2];

    // fd is intended to be blocking on the write side to ensure that signals
    // are delivered.
    if (pipe(pipefd) == -1)
        throw error_errno("pipe");

    read_pipe_ = pipefd[0];
    write_pipe_ = pipefd[1];

    set_fd(write_pipe_);
}

void SignalSource::setupSignalHandler() {
    prev_handler_ = ::signal(signum_, handle);

    if (prev_handler_ == SIG_ERR)
        throw error_errno("signal");
}

void SignalSource::removeSignalHandler() {
    ::signal(signum_, prev_handler_);
    prev_handler_ = SIG_DFL;

}

void SignalSource::closePipe() {
    clear_fd();

    close(write_pipe_);
    write_pipe_ = -1;

    close(read_pipe_);
    read_pipe_ = -1;
}
