#pragma once

#include <array>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace P2p
{

//! Exception thrown for a deserialization error.
class DeserializationError : public std::runtime_error
{
public:
    DeserializationError() : std::runtime_error("deserialization error") {}
};

//! An abstract class that enables an object to be serialized by the serialization functions.
class Serializable
{
public:

    //! Serializes the object
    virtual void serialize(std::vector<uint8_t> & out) const = 0;
};

/******************************************************************************************************************/
/*                                           S E R I A L I Z A T I O N                                            */
/******************************************************************************************************************/

//! Serializes a Serializable.
//!
//! @param  a       object to be serialized
//! @param  out     destination
template <typename T>
void serialize(T const & a, std::vector<uint8_t> & out)
{
    a.serialize(out);
}

//! Serializes a uint8_t.
//!
//! @param  a       value to be serialized
//! @param  out     destination
template <> void serialize<uint8_t>(uint8_t const & a, std::vector<uint8_t> & out);

//! Serializes a uint16_t.
//!
//! @param  a       value to be serialized
//! @param  out     destination
template <> void serialize<uint16_t>(uint16_t const & a, std::vector<uint8_t> & out);

//! Serializes a uint32_t.
//!
//! @param  a       value to be serialized
//! @param  out     destination
template <> void serialize<uint32_t>(uint32_t const & a, std::vector<uint8_t> & out);

//! Serializes a uint64_t.
//!
//! @param  a       value to be serialized
//! @param  out     destination
template <> void serialize<uint64_t>(uint64_t const & a, std::vector<uint8_t> & out);

//! Serializes an std::vector.
//!
//! @param  v       std::vector to be serialized
//! @param  out     destination
template <typename T>
void serializeVector(std::vector<T> const & v, std::vector<uint8_t> & out)
{
    for (auto const & element : v)
    {
        serialize(element, out);
    }
}

//! Serializes a vector of std::array.
//!
//! @param  v       std::vector to be serialized
//! @param  out     destination
template <typename T, size_t N>
void serializeVector(std::vector<std::array<T, N> > const & v, std::vector<uint8_t> & out)
{
    for (auto const & element : v)
    {
        serializeArray(element, out);
    }
}

//! Serializes a vector of uint8_t.
//!
//! @param  a       std::vector to be serialized
//! @param  out     destination
template <> void serializeVector<uint8_t>(std::vector<uint8_t> const & a, std::vector<uint8_t> & out);

//! A helper class for serialization of an std::array.
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

//! A helper class for serialization of an std::array of uint8_t
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

//! Serializes an std::array.
//!
//! @param  a       std::array to be serialized
//! @param  out     destination
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

//! Deserializes an object with a deserialization constructor.
//!
//! @param[in,out]  in      pointer to the next byte to deserialize
//! @param[in,out]  size    number of bytes remaining in the serialized stream
//!
//! @note   The object being deserialized must have a constructor of the form T(uint8_t const * & in, size_t & size)
template <typename T>
T deserialize(uint8_t const * & in, size_t & size)
{
    return T(in, size);
}

//! Deserializes a uint8_t.
//!
//! @param[in,out]  in      pointer to the next byte to deserialize
//! @param[in,out]  size    number of bytes remaining in the serialized stream
template <> uint8_t deserialize<uint8_t>(uint8_t const * & in, size_t & size);

//! Deserializes a uint16_t.
//!
//! @param[in,out]  in      pointer to the next byte to deserialize
//! @param[in,out]  size    number of bytes remaining in the serialized stream
template <> uint16_t deserialize<uint16_t>(uint8_t const * & in, size_t & size);

//! Deserializes a uint32_t.
//!
//! @param[in,out]  in      pointer to the next byte to deserialize
//! @param[in,out]  size    number of bytes remaining in the serialized stream
template <> uint32_t deserialize<uint32_t>(uint8_t const * & in, size_t & size);

//! Deserializes a uint64_t.
//!
//! @param[in,out]  in      pointer to the next byte to deserialize
//! @param[in,out]  size    number of bytes remaining in the serialized stream
template <> uint64_t deserialize<uint64_t>(uint8_t const * & in, size_t & size);

//! Deserializes a vector.
//!
//! @param          n       Number of elements to deserialize
//! @param[in,out]  in      pointer to the next byte to deserialize
//! @param[in,out]  size    number of bytes remaining in the serialized stream
//!
//! @note   The elements of the vector must support deserialization
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

//! Deserializes a vector of std::array.
//!
//! @param          n       Number of std::array to deserialize
//! @param[in,out]  in      pointer to the next byte to deserialize
//! @param[in,out]  size    number of bytes remaining in the serialized stream
//!
//! @note   The elements of the array must support deserialization
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

//! Deserializes a vector of uint8_t.
//!
//! @param          n       Number of uint8_t to deserialize
//! @param[in,out]  in      pointer to the next byte to deserialize
//! @param[in,out]  size    number of bytes remaining in the serialized stream
template <> std::vector<uint8_t> deserializeVector<uint8_t>(size_t n, uint8_t const * & in, size_t & size);

//! A helper class for deserialization of an std::array.
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

//! A helper class for deserialization of a std::array of uint8_t.
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

//! Deserializes an std::array.
//!
//! @param[in,out]  in      pointer to the next byte to deserialize
//! @param[in,out]  size    number of bytes remaining in the serialized stream
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

//! A compressed 64-bit value.
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

//! An array of objects.
//!
//! This is used for serialization of an array of objects along with the number of elements in the array.
//!
//! @sa VASize

template <typename T>
class VarArray : public Serializable
{
public:
    // Constructor
    VarArray() {}

    // Constructor
    //!
    //! @param  v       The elements to be contained in the array
    explicit VarArray(std::vector<T> const & v) : data_(v) {}

    // Deserializaton constructor
    //! 
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    VarArray(uint8_t const * & in, size_t & size)
    {
        VASize arraySize(in, size);
        data_ = P2p::deserializeVector<T>(arraySize.value(), in, size);
    }
    
    virtual void serialize(std::vector<uint8_t> & out) const override
    {
        P2p::serialize(VASize(data_.size()), out);
        P2p::serializeVector(data_, out);
    }

    //! Returns the elements contained in the array
    std::vector<T> value() const { return data_; }

private:
    std::vector<T> data_;
};

//! An array of std::array
//!
//! This is used for serialization of an array of std::array along with the number of elements in the array.
//!
//! @sa VASize

template <typename T, size_t N>
class VarArray<std::array<T, N> > : public Serializable
{
public:
    // Constructor
    VarArray() {}

    // Constructor
    //!
    //! @param  v       The elements to be contained in the array
    explicit VarArray(std::vector<std::array<T, N> > const & v) : data_(v) {}

    // Deserializaton constructor
    //! 
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    VarArray(uint8_t const * & in, size_t & size)
    {
        VASize arraySize(in, size);
        data_ = P2p::deserializeVector<T, N>(arraySize.value(), in, size);
    }

    virtual void serialize(std::vector<uint8_t> & out) const override
    {
        P2p::serialize(VASize(data_.size()), out);
        P2p::serializeVector(data_, out);
    }

    //! Returns the elements contained in the array
    std::vector<std::array<T, N> > value() const { return data_; }

private:
    std::vector<std::array<T, N> > data_;
};

//! An array of uint8_t.
//!
//! This is primarily used for serialization of an array of uint8_t along with the number of elements in the array.
//!
//! @sa VASize

template <>
class VarArray<uint8_t> : public Serializable
{
public:
    // Constructor
    VarArray() {}

    // Constructor
    //!
    //! @param  v       The bytes to be contained in the array
    VarArray(std::vector<uint8_t> const & v) : data_(v) {}

    // Deserializaton constructor
    //! 
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    VarArray(uint8_t const * & in, size_t & size)
    {
        VASize arraySize(in, size);
        data_ = P2p::deserializeVector<uint8_t>(arraySize.value(), in, size);
    }

    virtual void serialize(std::vector<uint8_t> & out) const override
    {
        P2p::serialize(VASize(data_.size()), out);
        P2p::serializeVector(data_, out);
    }

    //! Returns the bytes contained in the array
    std::vector<uint8_t> value() const { return data_; }

private:
    std::vector<uint8_t> data_;
};

} // namespace Utility
