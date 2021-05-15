#include <cstring>

namespace shitty {

inline Payload::Payload(StreamBuf *target):
    target_(target)
{}

inline size_t Payload::size() const {
    return len_;
}

inline bool Payload::empty() const {
    return size() == 0;
}

inline const void* Payload::data() const {
    return data_.data();
}

inline void* Payload::data() {
    return data_.data();
}

inline void Payload::clear() {
    len_ = 0;
}

inline void Payload::flush() {
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
inline void Payload::write(const void *buf, size_t len) {
    if (size() + len > data_.size())
        flush();

    if (len > data_.size()) {
        target_->write(buf, len);
        return;
    }

    memcpy(data_.data() + len_, buf, len);
    len_ += len;
}

inline void Payload::write(const std::string& str) {
    write(str.data(), str.size());
}

inline void Payload::writeOctet(uint8_t octet) {
    write(&octet, 1);
}

inline void Payload::send(const void *buf, size_t len) {
    write(buf, len);
}

inline void Payload::send(const std::string& str) {
    write(str.data(), str.size());
}

inline size_t Payload::tailroom() const {
    return data_.size() - len_;
}

// TEST: that you can write exactly the size of the payload
// TEST: that you can make a single Payload::write() call that is bigger than
// the Payload's Capacity and it flushes existing data *then* sends the write.

}
