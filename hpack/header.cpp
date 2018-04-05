#include <cassert>
#include <cstring>
#include <stdexcept>

#include "huffy/decode.h"
#include "huffy/encode.h"

#include "header.h"
#include "number.h"

using std::string;
using std::vector;
using std::pair;

// TODO: Use `using u8string = std::basic_string<uint8_t>` or with C++17
// `using bstring = std::basic_string<std::byte>` instead of regular string with
// reinterpret_cast<>s.

struct static_table_entry {
    const char *name, *value;
};

// TODO: Replace these C-strings with C++-strings.
static const struct static_table_entry static_table[] = {
    { "", "" }, // 0 is not a valid index.
    { ":authority", "" },
    { ":method", "GET" },
    { ":method", "POST" },
    { ":path", "/" },
    { ":path", "/index.html" },
    { ":scheme", "http" },
    { ":scheme", "https" },
    { ":status", "200" },
    { ":status", "204" },
    { ":status", "206" },
    { ":status", "304" },
    { ":status", "400" },
    { ":status", "404" },
    { ":status", "500" },
    { "accept-charset", "" },
    { "accept-encoding", "gzip, deflate" },
    { "accept-language", "" },
    { "accept-ranges", "" },
    { "accept", "" },
    { "access-control-allow-origin", "" },
    { "age", "" },
    { "allow", "" },
    { "authorization", "" },
    { "cache-control", "" },
    { "content-disposition", "" },
    { "content-encoding", "" },
    { "content-language", "" },
    { "content-length", "" },
    { "content-location", "" },
    { "content-range", "" },
    { "content-type", "" },
    { "cookie", "" },
    { "date", "" },
    { "etag", "" },
    { "expect", "" },
    { "expires", "" },
    { "from", "" },
    { "host", "" },
    { "if-match", "" },
    { "if-modified-since", "" },
    { "if-none-match", "" },
    { "if-range", "" },
    { "if-unmodified-since", "" },
    { "last-modified", "" },
    { "link", "" },
    { "location", "" },
    { "max-forwards", "" },
    { "proxy-authenticate", "" },
    { "proxy-authorization", "" },
    { "range", "" },
    { "referer", "" },
    { "refresh", "" },
    { "retry-after", "" },
    { "server", "" },
    { "set-cookie", "" },
    { "strict-transport-security", "" },
    { "transfer-encoding", "" },
    { "user-agent", "" },
    { "vary", "" },
    { "via", "" },
    { "www-authenticate", "" },
};

enum class StaticHeader: uint_fast8_t {
    AUTHORITY = 1,
    METHOD_GET,
    METHOD_POST,
    PATH_SLASH,
    PATH_SLASH_INDEX_HTML,
    SCHEME_HTTP,
    SCHEME_HTTPS,
    STATUS_200,
    STATUS_204,
    STATUS_206,
    STATUS_304,
    STATUS_400,
    STATUS_404,
    STATUS_500,
    ACCEPT_CHARSET,
    ACCEPT_ENCODING_GZIP_DEFLATE,
    ACCEPT_LANGUAGE,
    ACCEPT_RANGES,
    ACCEPT,
    ACCESS_CONTROL_ALLOW_ORIGIN,
    AGE,
    ALLOW,
    AUTHORIZATION,
    CACHE_CONTROL,
    CONTENT_DISPOSITION,
    CONTENT_ENCODING,
    CONTENT_LANGUAGE,
    CONTENT_LENGTH,
    CONTENT_LOCATION,
    CONTENT_RANGE,
    CONTENT_TYPE,
    COOKIE,
    DATE,
    ETAG,
    EXPECT,
    EXPIRES,
    FROM,
    HOST,
    IF_MATCH,
    IF_MODIFIED_SINCE,
    IF_NONE_MATCH,
    IF_RANGE,
    IF_UNMODIFIED_SINCE,
    LAST_MODIFIED,
    LINK,
    LOCATION,
    MAX_FORWARDS,
    PROXY_AUTHENTICATE,
    PROXY_AUTHORIZATION,
    RANGE,
    REFERER,
    REFRESH,
    RETRY_AFTER,
    SERVER,
    SET_COOKIE,
    STRICT_TRANSPORT_SECURITY,
    TRANSFER_ENCODING,
    USER_AGENT,
    VARY,
    VIA,
    WWW_AUTHENTICATE,
};


Header::Header(const string& n, const string& v):
    name_(n),
    value_(v)
{}

Header::Header(string&& n, string&& v):
    name_(std::move(n)),
    value_(std::move(v))
{}

InternalHeader::InternalHeader(
        const std::string& name,
        const std::string& value,
        bool never_indexed):
    InternalHeader(
            name,
            value,
            string(),
            string(),
            never_indexed)
{}

InternalHeader::InternalHeader(
        const std::string& name,
        const std::string& value,
        const std::string& name_encoded,
        const std::string& value_encoded,
        bool never_indexed):
    name_(name),
    value_(value),
    name_encoded_(name_encoded),
    value_encoded_(value_encoded),
    never_indexed_(never_indexed)
{}

InternalHeader::InternalHeader(
        std::string&& name,
        std::string&& value,
        std::string&& name_encoded,
        std::string&& value_encoded,
        bool never_indexed):
    name_(std::move(name)),
    value_(std::move(value)),
    name_encoded_(std::move(name_encoded)),
    value_encoded_(std::move(value_encoded)),
    never_indexed_(never_indexed)
{}

InternalHeader::InternalHeader(const Header& h):
    name_(h.name()),
    value_(h.value()),
    name_encoded_(),
    value_encoded_(),
    never_indexed_(false)
{}

Header
InternalHeader::header() const {
    return Header(name_, value_);
}

unsigned
InternalHeader::hpack_size() const {
    return 32 + name_.size() + value_.size();
}

void
InternalHeader::value(const std::string& v, const std::string& encoded_value) {
    value_ = v;
    value_encoded_ = encoded_value;
}

void
InternalHeader::value(std::string&& v, std::string&& encoded_value) {
    value_ = std::move(v);
    value_encoded_ = std::move(encoded_value);
}

// This might turn out to be a useful general purpose function.
static string hpack_encode_string(const string& s) {
    string coded(10 + s.size(), '\0');

    ssize_t len = huffman_encode(
            reinterpret_cast<const uint8_t*>(s.data()),
            s.size(),
            reinterpret_cast<uint8_t*>(&coded[0]),
            coded.size());

    // Determining which one is longer is a bit tricky; huffman_encode()
    // includes the length prefix in its encoded version so we might need to
    // figure out how long the length prefix on the raw version is before
    // determining which is shorter.

    if (len > 0) {
        coded.resize(len); // truncate to actual length
        coded.shrink_to_fit();

        if (coded.size() <= s.size()) {
            // Coded version is smaller even including its length prefix
            // (if they're currently equal, adding a length prefix to the raw
            // string would make it longer)
            return coded;
        }
    }

    // Either encode failed (len < 0) or not sure which will be shorter.
    uint8_t raws_length_prefix[10];

    ssize_t raws_length_prefix_length = encode_number(s.size(), 7,
            raws_length_prefix, sizeof(raws_length_prefix));

    if (raws_length_prefix_length < 0) {
        // Failed to encode the length prefix :)
        if (len > 0) // Huffman coding worked at least
            return coded;
        else
            throw std::runtime_error("Failed to encode string");
    }

    unsigned raw_with_length_prefix_length = raws_length_prefix_length + s.size();
    if (len < 0 || raw_with_length_prefix_length < len) {
        // Use raw. But reusing the `coded` buffer since it's already allocated.
        // (Returned string needs a length prefix)
        coded.resize(raw_with_length_prefix_length);
        coded.shrink_to_fit();

        memcpy(&coded[0], raws_length_prefix, raws_length_prefix_length);
        memcpy(&coded[raws_length_prefix_length], s.data(), s.size());

        return coded;
    }

    // Huffcoded version is shorter
    return coded;
}

const string&
InternalHeader::name_encoded() const {
    if (!name_encoded_.empty())
        return name_encoded_;

    name_encoded_ = hpack_encode_string(name_);
    return name_encoded_;
}

const string&
InternalHeader::value_encoded() const {
    if (!value_encoded_.empty())
        return value_encoded_;

    value_encoded_ = hpack_encode_string(name_);
    return value_encoded_;
}

DynamicTable::DynamicTable() {
    table_.reserve(64);
}

InternalHeader
DynamicTable::get(unsigned index) const {
    if (index < start_)
        throw std::out_of_range("requested static table index from dynamic table");

    index -= start_;

    if (index >= table_.size())
        throw std::out_of_range("dynamic table index out of range");

    return table_[index];
}

// Size of the table in octets according to HPACK.
// ie. For each header in the table, the number of octets in the
// non-Huffman-coded key & value, plus 32.
// FUTURE: this could be tracked as a running total, updated on
// add/remove.
unsigned
DynamicTable::hpack_size() const {
    unsigned s = 0;
    for (const InternalHeader& header: table_) {
        s += header.hpack_size();
    }
    return s;
}

void
DynamicTable::insert(const InternalHeader& header) {
    InternalHeader h(header);
    return insert(std::move(h));
}

// XXX: This is probably only used via the const& version
void
DynamicTable::insert(InternalHeader&& header) {
    unsigned newh_size = header.hpack_size();

    if (newh_size > size_octets_) {
        // unlikely, but new header doesn't fit in the table at all
        table_.clear();
        return;
    }

    // XXX: This is horribly inefficient; O(n^2) in number of headers for
    // large inserts.
    // It can at least easily be changed to determine how many entries to
    // remove rather than just removing one at a time and recalculating
    // everything.
    while (hpack_size() + newh_size > size_octets_ && !table_.empty()) {
        table_.pop_back();
    }
    table_.insert(table_.begin(), std::move(header));
}

void
DynamicTable::resize(unsigned new_hpack_size) {
    if (new_hpack_size >= size_octets_) {
        size_octets_ = new_hpack_size;
        return;
    }

    size_t nremove = 0;
    unsigned hsize = hpack_size();

    while (!table_.empty() && hsize > new_hpack_size) {
        ++nremove;
        unsigned this_entry_sz = table_[table_.size() - nremove].hpack_size();
        assert(this_entry_sz > hsize);
        hsize -= this_entry_sz;
    }

    table_.erase(table_.begin() + (table_.size() - nremove), table_.end());
}

// TODO: DynamicTable::find()

Header
StaticTable::get(unsigned index) const {
    if (index == 0 || index > sizeof(static_table) / sizeof(static_table[0]))
        throw std::out_of_range("invalid index for static table");

    return Header(static_table[index].name, static_table[index].value);
}

// TODO StaticTable::find()

InternalHeader
HeaderTable::get(unsigned index) const {
    // StaticTable validates index == 0
    return index < 62 ? InternalHeader(stable_.get(index)) : dtable_.get(index);
}

// TODO: HeaderTable::find()
// If the static table has a key match but not a value match,
// search the dynamic table because it might have a k+v match.
// Or: Search the dynamic table first and just use its result?
// Also: Consider maintaining a map of keys to indices, at least for the static
// table, to aid quick lookups for the find() function.
// Also: This functionality is only useful for the sending side
// (encoding headers) and useless for the receiver to maintain,
// so maybe move it out into a SenderTable class ensuring that it doesn't
// add cruft and useless work to the decoder side.

void RBuf::advance(size_t len) {
    if (__builtin_expect(data_ + len > end_, 0))
        throw std::runtime_error("Cannot advance beyond end of buffer");

    data_ += len;
}

RBuf& RBuf::operator+=(size_t len) {
    advance(len);
    return *this;
}

InternalHeader HeaderDecoder::decode(RBuf& buf) {
    if (buf.empty())
        throw std::runtime_error("empty buffer");

    // The string will be at most 8÷5 the length of the encoded
    // representation (of course add 1 due to integer rounding).
    // So just decode the length, allocate and decode.
    if (*buf.data() & 0x80)
        return decode_indexed(buf);

    if (*buf.data() & 0x40)
        return decode_literal_incremental(buf);

    // dynamic_table_resize must be detected by the caller before calling this
    // function. (if ((byte[0] & 0x20 == 0x20)).)
    // XXX: Make sure this is the case ^.
    if ((*buf.data() & 0x20) == 0x20)
        throw std::logic_error("HeaderDecoder::decode() got a dynamic table resize call but isn't equipped to handle it");

    if (*buf.data() & 0x10)
        return decode_literal_never_indexed(buf);

    return decode_literal_without_indexing(buf);
}

InternalHeader HeaderDecoder::decode_indexed(RBuf& buf) {
    uintmax_t idx = 0;
    ssize_t len = decode_number(buf.data(), buf.size(), 7, &idx);
    if (len < 1)
        throw std::runtime_error("Failed to decode indexed header");

    buf.advance(len);

    return table_.get(idx);
}

static pair<string, string>
decode_raw_string_from_rbuf(RBuf& buf) {
    uintmax_t slen = 0;
    ssize_t bufsz = decode_number(buf.data(), buf.size(), 7, &slen);
    if (bufsz < 1)
        throw std::runtime_error("String length decode failure");

    string encoded(reinterpret_cast<const char*>(buf.data()), bufsz);

    buf.advance(bufsz);

    if (slen > buf.size())
        throw std::runtime_error("String prefix longer than buffer");

    encoded.append(reinterpret_cast<const char *>(buf.data()), slen);

    string s(reinterpret_cast<const char*>(buf.data()), slen);

    buf.advance(slen);

    return std::make_pair(s, encoded);
}

static pair<string, string>
decode_huff_string_from_rbuf(RBuf& buf) {
    uintmax_t coded_len = 0;
    ssize_t bufsz = decode_number(buf.data(), buf.size(), 7, &coded_len);
    if (bufsz < 1)
        throw std::runtime_error("Huffman-coded string length decode failure");

    string encoded(reinterpret_cast<const char *>(buf.data()), bufsz);
    buf.advance(bufsz);

    if (coded_len > buf.size())
        throw std::runtime_error("Huffman-coded string length longer than buffer");

    encoded.append(reinterpret_cast<const char *>(buf.data()), coded_len);

    ssize_t decoded_len = coded_len * 8 / 5 + 1;

    string s(decoded_len, '\0');

    // NB. huffman_decode API is a bit different; doesn't return how much buffer
    // was used (since it uses length-prefixing), it returns the amount of
    // output buffer used.
    decoded_len = huffman_decode(
            buf.data(),
            coded_len,
            8,
            reinterpret_cast<uint8_t*>(&s[0]),
            s.size());

    buf.advance(coded_len);

    // XXX: overflow check before cast.
    assert(decoded_len <= static_cast<ssize_t>(s.size()));

    s.resize(decoded_len);

    return make_pair(s, encoded);
}

static pair<string, string>
decode_string_from_rbuf(RBuf& buf) {
    if (buf.empty())
        throw std::runtime_error("End of buffer decoding string");

    return (*buf & 0x80)
        ? decode_huff_string_from_rbuf(buf)
        : decode_raw_string_from_rbuf(buf);
}

// A header with an indexed name and literal value.
InternalHeader HeaderDecoder::decode_literal_indexed(unsigned index, RBuf& buf) {
    InternalHeader h = table_.get(index);
    pair<string, string> value = decode_string_from_rbuf(buf);
    h.value(value.first, value.second);
    table_.insert(h);
    return h;
}

InternalHeader
HeaderDecoder::decode_literal(RBuf& buf, uint_fast8_t length_bits) {
    uintmax_t idx = 0;
    ssize_t len = decode_number(buf.data(), buf.size(), length_bits, &idx);

    if (len < 1)
        throw std::runtime_error("Failed to decode literal incremental header");

    buf.advance(len);

    if (idx) {
        InternalHeader h(decode_literal_indexed(idx, buf));
        // Header was already inserted into dtable
        return h;
    }

    // These must be separate calls to ensure that the name is sequenced before
    // the value, since the buffer is advanced by each decode call.
    pair<string, string> name = decode_string_from_rbuf(buf);
    pair<string, string> value = decode_string_from_rbuf(buf);

    InternalHeader h(
            std::move(name.first), std::move(value.first),
            std::move(name.second), std::move(value.second));

    return h;
}

InternalHeader HeaderDecoder::decode_literal_incremental(RBuf& buf) {
    InternalHeader h(decode_literal(buf, 6));
    table_.insert(h);
    return h;
}

void
HeaderDecoder::dynamic_table_resize(RBuf& buf) {
    if (buf.empty() || (*buf & 0x02) != 0x02)
        throw std::logic_error("Not a dynamic table size update message");

    uintmax_t new_size = 0;
    ssize_t len = decode_number(buf.data(), buf.size(), 5, &new_size);
    if (len < 1)
        throw std::runtime_error("Error decoding buffer resize message");

    buf.advance(len);

    table_.resize(new_size);
}

InternalHeader
HeaderDecoder::decode_literal_without_indexing(RBuf& buf) {
    return decode_literal(buf, 4);
}

InternalHeader
HeaderDecoder::decode_literal_never_indexed(RBuf& buf) {
    InternalHeader h(decode_literal(buf, 4));
    h.never_indexed(true);
    return h;
}
