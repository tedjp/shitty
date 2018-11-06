#pragma once

namespace shitty {

class EventReceiver {
public:
    virtual void onPollIn();
    virtual void onPollOut();
    virtual void onPollErr();
    virtual int getPollFD() const = 0;
};

}
