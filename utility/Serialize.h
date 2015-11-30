#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <array>

namespace Utility
{

//! Exception thrown by deserialization error
class DeserializationError : public std::runtime_error
{
public:
    DeserializationError() : std::runtime_error("deserialization error") {}
};

//! Serialization of a single object
template <typename T>
void serialize(T const & a, std::vector<uint8_t> & out)
{
    a.serialize(out);
}

//! Serialization of a single uint8_t
template <> void serialize<uint8_t>(uint8_t const & a, std::vector<uint8_t> & out);

//! Serialization of a single uint16_t
template <> void serialize<uint16_t>(uint16_t const & a, std::vector<uint8_t> & out);

//! Serialization of a single uint32_t
template <> void serialize<uint32_t>(uint32_t const & a, std::vector<uint8_t> & out);

//! Serialization of a single uint64_t
template <> void serialize<uint64_t>(uint64_t const & a, std::vector<uint8_t> & out);

//! Serialization of a vector
template <typename T>
void serializeVector(std::vector<T> const & a, std::vector<uint8_t> & out)
{
    for (auto const & element : a)
    {
        serialize(element, out);
    }
}

//! Serialization of a vector of uint8_t
template <> void serializeVector<uint8_t>(std::vector<uint8_t> const & a, std::vector<uint8_t> & out);

//! Deserialization of a single object
template <typename T>
T deserialize(uint8_t const * & in, size_t & size)
{
    return T(in, size);
}

//! Deserialization of a single uint8_t
template <> uint8_t  deserialize<uint8_t>(uint8_t const * & in, size_t & size);

//! Deserialization of a single uint16_t
template <> uint16_t deserialize<uint16_t>(uint8_t const * & in, size_t & size);

//! Deserialization of a single uint32_t
template <> uint32_t deserialize<uint32_t>(uint8_t const * & in, size_t & size);

//! Deserialization of a single uint64_t
template <> uint64_t deserialize<uint64_t>(uint8_t const * & in, size_t & size);

//! Deserialization of a vector
template <typename T>
std::vector<T> deserializeVector(size_t n, uint8_t const * & in, size_t & size)
{
    std::vector<T> v;
    v.reserve(n);
    for (size_t i = 0; i < n; ++i)
    {
        v.push_back(deserialize<T>(in, size));
    }
    return v;
}

//! Deserialization of a vector of uint8_t
template <> std::vector<uint8_t> deserializeVector<uint8_t>(size_t n, uint8_t const * & in, size_t & size);

// //! Deserialization of an std::array
// template <typename T, size_t N>
// std::array<T, N> deserializeArray<std::array<T, N> >(uint8_t const * & in, size_t & size)
// {
//     std::array<T, N> a;
//     for (auto & element : a)
//     {
//         element = deserialize<T>(in, size);
//     }
//     return a;
// }
// 
// //! Deserialization of a std::array of uint8_t
// template <size_t N>
// std::array<uint8_t, N> deserializeArray<std::array<uint8_t, N> >(uint8_t const * & in, size_t & size)
// {
//     if (size < N)
//     {
//         throw Utility::DeserializationError();
//     }
//     std::array<uint8_t, N> a;
//     std::copy(in, in + N, a.data());
//     in += N;
//     size -= N;
//     return a;
// }

//! @brief Compressed 64-bit value
//!
//! This value is primarily used in serializaton of arrays. In the reference code, it is known as CompactSize.
//!
//! @sa     VarArray

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

//! @brief Array of objects
//!
//! This array is primarily used for serialization of a vector of objects in certain cases.
//!
//! @sa VASize

template <typename T>
class VarArray
{
public:
    VarArray() {}
    VarArray(std::vector<T> const & v) : data_(v) {}
    VarArray(uint8_t const * & in, size_t & size)
    {
        VASize arraySize(in, size);
        data_ = deserializeVector<T>(arraySize.value(), in, size);
    }

    void serialize(std::vector<uint8_t> & out) const
    {
        VASize(data_.size()).serialize(out);
        serializeVector(data_, out);
    }

    std::vector<T> value() const { return data_; }
private:
    std::vector<T> data_;
};

//! @brief Array of uint8_t
//!
//! This array is primarily used for serialization of a vector of uint8_t in certain cases.
//!
//! @sa VASize

template <>
class VarArray<uint8_t>
{
public:
    VarArray(std::vector<uint8_t> const & v) : data_(v) {}
    VarArray(uint8_t const * & in, size_t & size)
    {
        VASize arraySize(in, size);
        data_ = deserializeVector<uint8_t>(arraySize.value(), in, size);
    }

    void serialize(std::vector<uint8_t> & out) const
    {
        VASize(data_.size()).serialize(out);
        serializeVector(data_, out);
    }

    std::vector<uint8_t> value() const { return data_; }
private:
    std::vector<uint8_t> data_;
};

//! @brief Array of std::arrays of objects
//!
//! This array is primarily used for serialization of a vector of std::arrays of objects in certain cases.
//!
//! @sa VASize

template <typename T, size_t N>
class VarArrayOfArrays
{
public:
    typedef std::array<T, N> A;
    VarArrayOfArrays() {}
    VarArrayOfArrays(std::vector<A> const & vofa) : data_(vofa) {}
    VarArrayOfArrays(uint8_t const * & in, size_t & size)
    {
        VASize vectorSize(in, size);
        data.reserve(vectorSize.value());
        for (size_t i = 0; i < vectorSize.value(); ++i)
        {
            data_.emplace_back(deserializeArray<T, N>(in, size));
        }
    }

    void serialize(std::vector<uint8_t> & out) const
    {
        VASize(data_.size()).serialize(out);
        for (auto const & a : data_)
        {
            serializeArray(a, out);
        }
    }

    std::vector<A> value() const { return data_; }
private:
    std::vector<A> data_;
};


} // namespace Utility
