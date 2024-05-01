#pragma once

#include <functional>

#include "Error.h"
#include "EventReceiver.h"
#include "SignalSource.h"

namespace shitty {

class SignalReceiver: public EventReceiver {
public:
    SignalReceiver(int signum, std::function<void(int)> action);

    int getPollFD() const override;

    void onPollIn() override;

private:
    SignalSource source_;
    std::function<void(int)> action_;
};

} // namespace shitty
