#pragma once

#include <string>
#include <vector>
#include <utility>

class Header {
private:
    std::string name_, value_;
public:
    Header(const std::string& n, const std::string& v);
    Header(std::string&& n, std::string&& v);
    const std::string& name() const { return name_; }
    const std::string& value() const { return value_; }
    void setValue(const std::string& v) { value_ = v; }
};

class InternalHeader {
private:
    std::string name_, value_, name_encoded_, value_encoded_;
    // Can't do this; indices change.
    //unsigned index; // 0 = not indexed
    bool never_indexed;
public:
    InternalHeader(
            const std::string& name,
            const std::string& value,
            bool never_indexed = false
            );
    InternalHeader(
            const std::string& name,
            const std::string& value,
            const std::string& name_encoded,
            const std::string& value_encoded,
            bool never_indexed = false
            );
    const std::string& name() const { return name_; }
    const std::string& value() const { return value_; }
    Header header() const;
    unsigned hpack_size() const;
};

class ITable {
public:
    virtual Header
        get(unsigned index) const = 0;

    // Look for the given header in the static table and return the index (or
    // zero if not found) and whether the value matches.
    // Used when encoding a header block to encode a header as an index.
    virtual std::pair<unsigned, bool>
        find(const Header& h) const = 0;
};

// For initial implementation just use vector::insert() at the beginning,
// but later on make it an adjustable-length ring buffer where only the
// start number (0 index) changes.
class DynamicTable: public ITable {
private:
    const unsigned start_ = 62; // RFC 7541
    unsigned size_octets_ = 4096; // RFC 7540
    std::vector<InternalHeader> table_;

    unsigned hpack_size() const;

public:
    DynamicTable();

    Header get(unsigned index) const override;
    std::pair<unsigned, bool> find(const Header& h) const override;
    void insert(InternalHeader&& header);
    void resize(unsigned new_hpack_size);
};

class StaticTable: public ITable {
public:
    StaticTable();

    Header
        get(unsigned index) const override;
    std::pair<unsigned, bool>
        find(const Header& h) const override;
};

class HeaderTable: public ITable {
private:
    StaticTable stable_;
    DynamicTable dtable_;

public:
    Header get(unsigned index) const override;
    std::pair<unsigned, bool>
        find(const Header& h) const override;
};

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

// Decode a literal, either raw or Huffman-coded.
std::string decode_string_from_rbuf(RBuf& rbuf);

class HeaderDecoder {
private:
    HeaderTable table_;

    // TODO: Implement
    std::vector<Header> parseHeaderFrame(RBuf& buf);

private:
    // XXX: Maybe make this private and make a separate public as below.
    Header decode(RBuf& buf);

    // § 6.1
    Header decode_indexed(RBuf& buf);
    // § 6.2.1
    Header decode_literal_incremental(RBuf& buf);
    // § 6.2.2
    Header decode_literal_without_indexing(RBuf& buf);
    // § 6.2.3
    Header decode_literal_never_indexed(RBuf& buf);
    // § 6.3
    void dynamic_table_resize(RBuf& buf);
};
