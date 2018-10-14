#include <cstdint>
#include <string>

namespace shitty::http2 {

typedef uint_fast32_t stream_id_t;

class Frame {
public:
//private:
    uint32_t length_; // 24-bits
    uint8_t type_;
    uint8_t flags_;
    stream_id_t stream_id_;
    // XXX: Maybe folly::IOBuf
    std::basic_string<uint8_t> payload_;
};
} // namespace shitty::http2
