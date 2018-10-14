#pragma once

#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <utility>

namespace shitty {

// Basically unique_ptr but for file descriptors

class SafeFD {
public:
    explicit SafeFD(int fd = -1):
        fd_(fd)
    {}

    SafeFD(const SafeFD&) = delete;
    SafeFD(SafeFD&& other) /*noexcept(std::is_nothrow_swappable<int>::value)*/ {
        ::std::swap(fd_, other.fd_);
    }
    SafeFD& operator=(const SafeFD&) = delete;
    SafeFD& operator=(SafeFD&& other) /*noexcept(std::is_nothrow_swappable<int>::value)*/ {
        ::std::swap(fd_, other.fd_);
        return *this;
    }

    // Implicit conversion to int for convenience
    operator int() const {
        return fd_;
    }

    ~SafeFD() {
        try {
            close();
        } catch (...)
        {}
    }

    int release() {
        int fd = fd_;
        fd_ = -1;
        return fd;
    }

    void reset() {
        close();
    }

    void swap(SafeFD& other) {
        std::swap(fd_, other.fd_);
    }

    explicit operator bool() const {
        return fd_ != -1;
    }

    int get() const {
        return fd_;
    }

    int operator*() const {
        return fd_;
    }

    void close() {
        if (fd_ == -1)
            return;

        if (::close(fd_) == -1)
            throw std::runtime_error(std::string("close: ") + strerror(errno));

        fd_ = -1;
    }

private:
    int fd_ = -1;
};

} // namespace shitty
