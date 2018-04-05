#pragma once

#include <string>
#include <vector>
#include <utility>

class Header {
private:
    std::string name_, value_;
    // Encoded values may be raw or Huffman coded, and always include the HPACK
    // length prefix.
    mutable std::string name_encoded_, value_encoded_;
    bool never_indexed_;
public:
    Header(
            const std::string& name,
            const std::string& value,
            const std::string& name_encoded = std::string(),
            const std::string& value_encoded = std::string(),
            bool never_indexed = false
            );
    Header(
            std::string&& name,
            std::string&& value,
            std::string&& name_encoded = std::string(),
            std::string&& value_encoded = std::string(),
            bool never_indexed = false
            );

    const std::string& name() const { return name_; }
    const std::string& value() const { return value_; }
    void value(const std::string& v, const std::string& encoded_value = std::string());
    void value(std::string&& v, std::string&& encoded_value = std::string());
    Header header() const;
    unsigned hpack_size() const;
    const std::string& name_encoded() const;
    const std::string& value_encoded() const;
    bool never_indexed() const { return never_indexed_; }
    void never_indexed(bool v) { never_indexed_ = v; }
};

// For initial implementation just use vector::insert() at the beginning,
// but later on make it an adjustable-length ring buffer where only the
// start number (0 index) changes.
class DynamicTable {
private:
    static const unsigned start_ = 62; // RFC 7541
    unsigned size_octets_ = 4096; // RFC 7540
    std::vector<Header> table_;

    unsigned hpack_size() const;

public:
    DynamicTable();

    Header get(unsigned index) const;
    std::pair<unsigned, bool> find(const Header& h) const;
    void insert(Header&& header);
    void insert(const Header& header);
    void resize(unsigned new_hpack_size);
};

class StaticTable {
public:
    StaticTable();

    Header get(unsigned index) const;
    std::pair<unsigned, bool>
        find(const Header& h) const;
};

class HeaderTable {
private:
    StaticTable stable_;
    DynamicTable dtable_;

public:
    Header get(unsigned index) const;
    std::pair<unsigned, bool>
        find(const Header& h) const;
    void insert(const Header& h) { dtable_.insert(h); }
    void resize(unsigned new_hpack_size) { dtable_.resize(new_hpack_size); }
};

// A *non-owning* interface to a contiguous buffer.
class RBuf {
private:
    const uint8_t *data_, *end_;
public:
    RBuf(const uint8_t *ptr, size_t size):
        data_(ptr),
        end_(ptr + size)
    {}

    const uint8_t *data() const { return data_; }
    size_t size() const { return end_ - data_; }
    bool empty() const { return data_ == end_; }

    void advance(size_t len);
    RBuf& operator+=(size_t len);
    uint8_t operator*() const { return data_[0]; }
};

class HeaderDecoder {
private:
    HeaderTable table_;

    std::vector<Header> parseHeaders(RBuf& buf);

private:
    // These need to return an Header in case the header was a
    // never-indexed header and you're writing a proxy that's going to forward
    // it.
    Header decode(RBuf& buf);

    // § 6.1
    Header decode_indexed(RBuf& buf);
    // § 6.2.1
    Header decode_literal_incremental(RBuf& buf);
    // Helpers
    Header decode_literal(RBuf& buf, uint_fast8_t length_bits);
    Header decode_literal_indexed(unsigned index, RBuf& buf);
    // § 6.2.2
    Header decode_literal_without_indexing(RBuf& buf);
    // § 6.2.3
    Header decode_literal_never_indexed(RBuf& buf);
    // § 6.3
    void dynamic_table_resize(RBuf& buf);
};
