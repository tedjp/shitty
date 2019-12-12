#pragma once

#include <signal.h>

#include "EventReceiver.h"

namespace shitty {

class SignalSource {
public:
    // TODO: Accept multiple signal numbers
    // Perhaps add()/remove() funcs
    SignalSource(int signum);
    ~SignalSource();

    int getPollFD() const;

private:
    void setupPipe();
    void setupSignalHandler();

    void removeSignalHandler();
    void closePipe();

    int signum_ = -1;
    sighandler_t prev_handler_ = nullptr;
    int read_pipe_ = -1;
    int write_pipe_ = -1;
};

} // namespace shitty
