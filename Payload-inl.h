#include <cstring>

namespace shitty {

template <size_t Capacity>
Payload<Capacity>::Payload(StreamBuf *target):
    target_(target)
{}

template <size_t Capacity>
size_t Payload<Capacity>::size() const {
    return len_;
}

template <size_t Capacity>
bool Payload<Capacity>::empty() const {
    return size() == 0;
}

template <size_t Capacity>
const void* Payload<Capacity>::data() const {
    return data_.data();
}

template <size_t Capacity>
void* Payload<Capacity>::data() {
    return data_.data();
}

template <size_t Capacity>
void Payload<Capacity>::clear() {
    len_ = 0;
}

template <size_t Capacity>
void Payload<Capacity>::flush() {
#if 0 // nice API
    target_->write(buf_.data(), size());
#else
    target_->ensure(size());
    memcpy(target_->tail(), data_.data(), size());
    target_->addTailContent(size());
    clear();
#endif
}

// [`buf`, `buf + len`) must not point to memory within this Payload
// (write() uses memcpy ()which is not overlap-safe.)
template <size_t Capacity>
void Payload<Capacity>::write(const void *buf, size_t len) {
    if (size() + len > data_.size())
        flush();

    if (len > data_.size()) {
        target_->write(buf, len);
        return;
    }

    memcpy(data_.data() + len_, buf, len);
    len_ += len;
}

template <size_t Capacity>
void Payload<Capacity>::write(const std::string& str) {
    write(str.data(), str.size());
}

template <size_t Capacity>
void Payload<Capacity>::send(const void *buf, size_t len) {
    write(buf, len);
}

template <size_t Capacity>
void Payload<Capacity>::send(const std::string& str) {
    write(str.data(), str.size());
}

template <size_t Capacity>
size_t Payload<Capacity>::tailroom() const {
    return data_.size() - len_;
}

// TEST: that you can write exactly the size of the payload
// TEST: that you can make a single Payload::write() call that is bigger than
// the Payload's Capacity and it flushes existing data *then* sends the write.

}
