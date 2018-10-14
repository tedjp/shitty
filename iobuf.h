#include <cstdint>
#include <memory>
#include <string>

namespace shitty {

// Low-budget clone of folly::IOBuf

typedef std::basic_string<uint8_t> DataString;

class IOBuf {
public:
    explicit IOBuf(size_t capacity = 0):
        start_(nullptr),
        end_(nullptr),
        data_(std::make_shared<std::string>(capacity, 0u))
    {}

    size_t size() const {
        return end_ - start_;
    }

    bool empty() const {
        return size() == 0;
    }

private:
    uint8_t *start_ = nullptr, *end_ = nullptr;

    std::shared_ptr<std::string> data_;

};

} // namespace shitty
