#pragma once

#include <array>
#include <cstdint>
#include <stdexcept>
#include <vector>

//! Exception thrown by deserialization error
class DeserializationError : public std::runtime_error
{
public:
    DeserializationError() : std::runtime_error("deserialization error") {}
};

class Serializable
{
public:

    //! Serializes the object
    virtual void serialize(std::vector<uint8_t> & out) const = 0;

};

namespace Utility
{

/******************************************************************************************************************/
/*                                           S E R I A L I Z A T I O N                                            */
/******************************************************************************************************************/

//! Serialization of a single object
template <typename T>
void serialize(T const & a, std::vector<uint8_t> & out)
{
    a.serialize(out);
}

//! Serialization of a single uint8_t
template <>
void serialize<uint8_t>(uint8_t const & a, std::vector<uint8_t> & out);

//! Serialization of a single uint16_t
template <>
void serialize<uint16_t>(uint16_t const & a, std::vector<uint8_t> & out);

//! Serialization of a single uint32_t
template <>
void serialize<uint32_t>(uint32_t const & a, std::vector<uint8_t> & out);

//! Serialization of a single uint64_t
template <>
void serialize<uint64_t>(uint64_t const & a, std::vector<uint8_t> & out);

//! Serialization of a vector
template <typename T>
void serializeVector(std::vector<T> const & v, std::vector<uint8_t> & out)
{
    for (auto const & element : v)
    {
        serialize(element, out);
    }
}

//! Serialization of a vector of std::array
template <typename T, size_t N>
void serializeVector(std::vector<std::array<T, N> > const & v, std::vector<uint8_t> & out)
{
    for (auto const & element : v)
    {
        serializeArray(element, out);
    }
}

//! Serialization of a vector of uint8_t
template <>
void serializeVector<uint8_t>(std::vector<uint8_t> const & a, std::vector<uint8_t> & out);

//! @brief  Serialization of an std::array helper class
//!
//! @note   The reason for this ridiculous code is that partial template specialization is not allowed with functions
template <typename T, size_t N>
struct SerializeArrayImpl
{
    void operator ()(std::array<T, N> const & a, std::vector<uint8_t> & out)
    {
        for (auto const & element : a)
        {
            serialize(element, out);
        }
    }

};

//! @brief  Serialization of a std::array of uint8_t helper class
//!
//! @note   The reason for this ridiculous code is that partial template specialization is not allowed with functions
template <size_t N>
struct SerializeArrayImpl<uint8_t, N>
{
    void operator ()(std::array<uint8_t, N> const & a, std::vector<uint8_t> & out)
    {
        out.insert(out.end(), a.begin(), a.end());
    }

};

//! @brief  Serialization of an std::array
//!
//! @note   The reason for this ridiculous code is that partial template specialization is not allowed with functions
template <typename T>
void serializeArray(T const & a, std::vector<uint8_t> & out)
{
    SerializeArrayImpl<T::value_type, std::tuple_size<T>::value> impl;
    impl(a, out);
}

/******************************************************************************************************************/
/*                                         D E S E R I A L I Z A T I O N                                          */
/******************************************************************************************************************/

//! Deserialization of a single object
template <typename T>
T deserialize(uint8_t const * & in, size_t & size)
{
    return T(in, size);
}

//! Deserialization of a single uint8_t
template <>
uint8_t deserialize<uint8_t>(uint8_t const * & in, size_t & size);

//! Deserialization of a single uint16_t
template <>
uint16_t deserialize<uint16_t>(uint8_t const * & in, size_t & size);

//! Deserialization of a single uint32_t
template <>
uint32_t deserialize<uint32_t>(uint8_t const * & in, size_t & size);

//! Deserialization of a single uint64_t
template <>
uint64_t deserialize<uint64_t>(uint8_t const * & in, size_t & size);

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

//! Deserialization of a vector of arrays
template <typename T, size_t N>
std::vector<std::array<T, N> > deserializeVector(size_t n, uint8_t const * & in, size_t & size)
{
    typedef std::array<T, N> A;

    std::vector<A> v;
    v.reserve(n);
    for (size_t i = 0; i < n; ++i)
    {
        v.push_back(deserializeArray<A>(in, size));
    }
    return v;
}

//! Deserialization of a vector of uint8_t
template <>
std::vector<uint8_t> deserializeVector<uint8_t>(size_t n, uint8_t const * & in, size_t & size);

//! @brief  Deserialization of an std::array helper class
//!
//! @note   The reason for this ridiculous code is that partial template specialization is not allowed with functions
template <typename T>
struct DeserializeArrayImpl
{
    T operator ()(uint8_t const * & in, size_t & size)
    {
        T a;
        for (auto & element : a)
        {
            element = deserialize<T>(in, size);
        }
        return a;
    }

};

//! @brief  Deserialization of a std::array of uint8_t helper class
//!
//! @note   The reason for this ridiculous code is that partial template specialization is not allowed with functions
template <size_t N>
struct DeserializeArrayImpl<std::array<uint8_t, N> >
{
    std::array<uint8_t, N> operator ()(uint8_t const * & in, size_t & size)
    {
        if (size < N)
            throw DeserializationError();
        std::array<uint8_t, N> a;
        std::copy(in, in + N, a.data());
        in += N;
        size -= N;
        return a;
    }

};

//! @brief  Deserialization of a std::array
//!
//! @note   The reason for this ridiculous code is that partial template specialization is not allowed with functions
template <typename T>
T deserializeArray(uint8_t const * & in, size_t & size)
{
    DeserializeArrayImpl<T> impl;
    return impl(in, size);
}

/******************************************************************************************************************/
/*                                          V A R I A B L E   A R R A Y                                           */
/******************************************************************************************************************/

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
//! This array is used for serialization of an array of objects along with the number of elements in the array.
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
        data_ = Utility::deserializeVector<T>(arraySize.value(), in, size);
    }

    void serialize(std::vector<uint8_t> & out) const
    {
        Utility::serialize(VASize(data_.size()), out);
        Utility::serializeVector(data_, out);
    }

    std::vector<T> value() const { return data_; }

private:
    std::vector<T> data_;
};

//! @brief Array of std::array
//!
//! This array is used for serialization of an array of std::array along with the number of elements in the array.
//!
//! @sa VASize

template <typename T, size_t N>
class VarArray<std::array<T, N> >
{
public:
    VarArray() {}
    VarArray(std::vector<std::array<T, N> > const & v) : data_(v) {}
    VarArray(uint8_t const * & in, size_t & size)
    {
        VASize arraySize(in, size);
        data_ = Utility::deserializeVector<T, N>(arraySize.value(), in, size);
    }

    void serialize(std::vector<uint8_t> & out) const
    {
        Utility::serialize(VASize(data_.size()), out);
        Utility::serializeVector(data_, out);
    }

    std::vector<std::array<T, N> > value() const { return data_; }

private:
    std::vector<std::array<T, N> > data_;
};

//! @brief  Array of uint8_t
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
        data_ = Utility::deserializeVector<uint8_t>(arraySize.value(), in, size);
    }

    void serialize(std::vector<uint8_t> & out) const
    {
        Utility::serialize(VASize(data_.size()), out);
        Utility::serializeVector(data_, out);
    }

    std::vector<uint8_t> value() const { return data_; }
private:
    std::vector<uint8_t> data_;
};

} // namespace Utility
