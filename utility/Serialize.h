#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

namespace Utility
{

// Exception thrown by deserialization error

class DeserializationError : public std::runtime_error
{
public:
    DeserializationError() : std::runtime_error("deserialization error") {}
};

// Serialization of a single value

template <typename T>
void serialize(T const & a, std::vector<uint8_t> & out)
{
    a.serialize(out);
}

template <> void serialize<uint8_t>(uint8_t const & a, std::vector<uint8_t> & out);

template <> void serialize<uint16_t>(uint16_t const & a, std::vector<uint8_t> & out);

template <> void serialize<uint32_t>(uint32_t const & a, std::vector<uint8_t> & out);

template <> void serialize<uint64_t>(uint64_t const & a, std::vector<uint8_t> & out);

// Serialization of an array

template <typename T>
void serializeArray(std::vector<T> const & a, std::vector<uint8_t> & out)
{
    for (auto const & element : a)
    {
        serialize(element, out);
    }
}

template <> void serializeArray<uint8_t>(std::vector<uint8_t> const & a, std::vector<uint8_t> & out);

// Deserialization of a single value

template <typename T>
T deserialize(uint8_t const * & in, size_t & size)
{
    return T(in, size);
}

template <> uint8_t  deserialize<uint8_t>(uint8_t const * & in, size_t & size);

template <> uint16_t deserialize<uint16_t>(uint8_t const * & in, size_t & size);

template <> uint32_t deserialize<uint32_t>(uint8_t const * & in, size_t & size);

template <> uint64_t deserialize<uint64_t>(uint8_t const * & in, size_t & size);

// Deserialization of an array

template <typename T>
std::vector<T> deserializeArray(size_t n, uint8_t const * & in, size_t & size)
{
    std::vector<T> v;
    v.reserve(n);
    for (size_t i = 0; i < n; ++i)
    {
        v.push_back(deserialize<T>(in, size));
    }
    return v;
}

template <> std::vector<uint8_t> deserializeArray<uint8_t>(size_t n, uint8_t const * & in, size_t & size);

class VASize
{
public:
    VASize(uint64_t v) : value_(v) {}
    VASize(uint8_t const * & in, size_t & size);
    void serialize(std::vector<uint8_t> & out) const;

    uint64_t value() const { return value_; }
private:
    uint64_t value_;
};

template <typename T>
class VarArray
{
public:
    VarArray() {}
    VarArray(std::vector<T> const & v) : data_(v) {}
    VarArray(uint8_t const * & in, size_t & size)
    {
        VASize arraySize(in, size);
        data_ = deserializeArray<T>(arraySize.value(), in, size);
    }

    void serialize(std::vector<uint8_t> & out) const
    {
        VASize(data_.size()).serialize(out);
        serializeArray(data_, out);
    }

    std::vector<T> value() const { return data_; }
private:
    std::vector<T> data_;
};

template <>
class VarArray<uint8_t>
{
public:
    VarArray(std::vector<uint8_t> const & v) : data_(v) {}
    VarArray(uint8_t const * & in, size_t & size)
    {
        VASize arraySize(in, size);
        data_ = deserializeArray<uint8_t>(arraySize.value(), in, size);
    }

    void serialize(std::vector<uint8_t> & out) const
    {
        VASize(data_.size()).serialize(out);
        serializeArray(data_, out);
    }

    std::vector<uint8_t> value() const { return data_; }
private:
    std::vector<uint8_t> data_;
};

} // namespace Utility
