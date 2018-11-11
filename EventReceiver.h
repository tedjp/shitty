#pragma once

namespace shitty {

class EventReceiver {
public:
    // As currently implemented, it's not safe to move an EventReceiver because
    // it won't update the object pointer in epoll.
    EventReceiver() = default;
    EventReceiver(const EventReceiver&) = delete;
    EventReceiver(EventReceiver&&) = delete;
    EventReceiver& operator=(const EventReceiver&) = delete;
    EventReceiver& operator=(EventReceiver&&) = delete;
    virtual ~EventReceiver();

    // TODO: Make noncopyable non-dleteabed.
    virtual void onPollIn();
    virtual void onPollOut();
    virtual void onPollErr();
    virtual int getPollFD() const = 0;
};

} // namespace shitty
