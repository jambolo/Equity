#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <utility/Utility.h>
#include <vector>

//! @todo Move serialization to Serializer class in Network namespace

namespace P2p
{
//! @ingroup P2pGroup
//!@{

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

    virtual ~Serializable() {}
    //! Serializes the object.
    //!
    //! @param[out]     out     destination
    //!
    //! @note   Must be overridden
    virtual void serialize(std::vector<uint8_t> & out) const = 0;

    //! Converts the object to a JSON object.
    //!
    //! @note   Must be overridden
    virtual nlohmann::json toJson() const = 0;
};

/******************************************************************************************************************/
/*                                           S E R I A L I Z A T I O N                                            */
/******************************************************************************************************************/

//! Serializes a Serializable.
//!
//! @param  a       object to be serialized
//! @param  out     destination
void serialize(Serializable const & a, std::vector<uint8_t> & out);

//! Serializes a uint8_t.
//!
//! @param  a       value to be serialized
//! @param  out     destination
void serialize(uint8_t const & a, std::vector<uint8_t> & out);

//! Serializes a uint16_t.
//!
//! @param  a       value to be serialized
//! @param  out     destination
void serialize(uint16_t const & a, std::vector<uint8_t> & out);

//! Serializes a uint32_t.
//!
//! @param  a       value to be serialized
//! @param  out     destination
void serialize(uint32_t const & a, std::vector<uint8_t> & out);

//! Serializes a uint64_t.
//!
//! @param  a       value to be serialized
//! @param  out     destination
void serialize(uint64_t const & a, std::vector<uint8_t> & out);

//! Serializes an std::string.
//!
//! @param  s       std::string to be serialized
//! @param  out     destination
void serialize(std::string const & s, std::vector<uint8_t> & out);

//! Serializes a string.
//!
//! @param  s       string to be serialized
//! @param  out     destination
void serialize(char const * s, std::vector<uint8_t> & out);

//! Serializes an std::vector.
//!
//! @param  v       std::vector to be serialized
//! @param  out     destination
template <typename T> void serialize(std::vector<T> const & v, std::vector<uint8_t> & out);

//! Serializes a vector of std::array.
//!
//! @param  v       std::vector to be serialized
//! @param  out     destination
template <typename T, size_t N> void serialize(std::vector<std::array<T, N>> const & v, std::vector<uint8_t> & out);

//! Serializes a vector of uint8_t.
//!
//! @param  a       std::vector to be serialized
//! @param  out     destination
template <> void serialize<uint8_t>(std::vector<uint8_t> const & a, std::vector<uint8_t> & out);

//! Serializes an std::array.
//!
//! @param  a       std::array to be serialized
//! @param  out     destination
//!
//! @internal   The reason for this ridiculous code is that partial template specialization is not allowed with functions
template <typename T, size_t N> void serialize(std::array<T, N> const & a, std::vector<uint8_t> & out);

inline void serialize(Serializable const & a, std::vector<uint8_t> & out)
{
    a.serialize(out);
}

template <typename T>
void serialize(std::vector<T> const & v, std::vector<uint8_t> & out)
{
    for (auto const & element : v)
    {
        serialize(element, out);
    }
}

template <typename T, size_t N>
void serialize(std::vector<std::array<T, N>> const & v, std::vector<uint8_t> & out)
{
    for (auto const & element : v)
    {
        serialize(element, out);
    }
}

//! A helper class for serialization of an std::array.
//!
//! @internal   The reason for this ridiculous code is that partial template specialization is not allowed with functions
template <typename T, size_t N>
struct SerializeArrayImpl
{
    //! Serialization functor.
    void operator ()(std::array<T, N> const & a, std::vector<uint8_t> & out)
    {
        for (auto const & element : a)
        {
            serialize(element, out);
        }
    }
};

//! A helper class for serialization of an std::array of uint8_t.
//!
//! @internal   The reason for this ridiculous code is that partial template specialization is not allowed with functions
template <size_t N>
struct SerializeArrayImpl<uint8_t, N>
{
    //! Serialization functor.
    void operator ()(std::array<uint8_t, N> const & a, std::vector<uint8_t> & out)
    {
        out.insert(out.end(), a.begin(), a.end());
    }
};

template <typename T, size_t N>
void serialize(std::array<T, N> const & a, std::vector<uint8_t> & out)
{
    SerializeArrayImpl<T, N> impl;
    impl(a, out);
}

/******************************************************************************************************************/
/*                                          J S O N   C O N V E R S I O N                                         */
/******************************************************************************************************************/

//! Converts a Serializable to JSON object.
//!
//! @param  x       object to be serialized
nlohmann::json toJson(Serializable const & x);

//! Converts a generic array to a JSON array value toJson.
//!
//! @param  v       pointer to array
//! @param  size    number of elements in the array
template <typename T> nlohmann::json toJson(T const * v, size_t size);

//! Converts a std::vector to a JSON array value toJson.
//!
//! @param  v       std::vector to be serialized
template <typename T> nlohmann::json toJson(std::vector<T> const & v);

//! Converts a std::vector<uint8_t> to a JSON hex string toJson.
//!
//! @param  v
template <> nlohmann::json toJson(std::vector<uint8_t> const & v);

//! Converts a generic std::array to a JSON array value
//!
//! @param  a       std::array to be serialized
template <typename T, size_t N> nlohmann::json toJson(std::array<T, N> const & a);

//! A helper class for converting an std::array to JSON.
//!
//! @internal   The reason for this ridiculous code is that partial template specialization is not allowed with functions
template <typename T, size_t N>
struct ToJsonArrayImpl
{
    //! Json functor.
    nlohmann::json operator ()(std::array<T, N> const & a)
    {
        return toJson(a.data(), a.size());
    }
};

//! A helper class for converting an std::array<uint8_t, N> to JSON.
//!
//! @internal   The reason for this ridiculous code is that partial template specialization is not allowed with functions
template <size_t N>
struct ToJsonArrayImpl<uint8_t, N>
{
    //! Json functor.
    nlohmann::json operator ()(std::array<uint8_t, N> const & a)
    {
        return Utility::toHex(a.data(), a.size());
    }
};

inline nlohmann::json toJson(Serializable const & x)
{
    return x.toJson();
}

template <typename T>
nlohmann::json toJson(T const * v, size_t size)
{
    nlohmann::json array;
    if (v)
    {
        while (size-- > 0)
        {
            array += toJson(*v++);
        }
    }
    return array;
}

template <typename T>
nlohmann::json toJson(std::vector<T> const & v)
{
    return toJson(v.data(), v.size());
}

template <typename T, size_t N>
nlohmann::json toJson(std::array<T, N> const & a)
{
    ToJsonArrayImpl<T, N> impl;
    return impl(a);
}

/******************************************************************************************************************/
/*                                          D E S E R I A L I Z A T I O N                                         */
/******************************************************************************************************************/

//! Deserializes an object with a deserialization constructor.
//!
//! @param[in,out]  in      pointer to the next byte to deserialize
//! @param[in,out]  size    number of bytes remaining in the serialized stream
//!
//! @note   The object being deserialized must have a constructor of the form T(uint8_t const * & in, size_t & size)
template <typename T> T deserialize(uint8_t const * & in, size_t & size);

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
template <typename T> std::vector<T> deserializeVector(size_t n, uint8_t const * & in, size_t & size);

//! Deserializes a vector of uint8_t.
//!
//! @param          n       Number of uint8_t to deserialize
//! @param[in,out]  in      pointer to the next byte to deserialize
//! @param[in,out]  size    number of bytes remaining in the serialized stream
template <> std::vector<uint8_t> deserializeVector<uint8_t>(size_t n, uint8_t const * & in, size_t & size);

//! Deserializes an std::array.
//!
//! @param[in,out]  in      pointer to the next byte to deserialize
//! @param[in,out]  size    number of bytes remaining in the serialized stream
//!
//! @note   The reason for this ridiculous code is that partial template specialization is not allowed with functions
template <typename T> T deserializeArray(uint8_t const * & in, size_t & size);

//! Deserializes a vector of std::array.
//!
//! @param          n       Number of std::array to deserialize
//! @param[in,out]  in      pointer to the next byte to deserialize
//! @param[in,out]  size    number of bytes remaining in the serialized stream
//!
//! @note   The elements of the array must support deserialization
template <typename T, size_t N> std::vector<std::array<T, N>> deserializeVector(size_t n, uint8_t const * & in, size_t & size);

//! Deserializes a string.
//!
//! @param          n       Number of characters to deserialize
//! @param[in,out]  in      pointer to the next byte to deserialize
//! @param[in,out]  size    number of bytes remaining in the serialized stream
std::string deserializeString(size_t n, uint8_t const * & in, size_t & size);

//! A helper class for deserialization of an std::array.
//!
//! @note   The reason for this ridiculous code is that partial template specialization is not allowed with functions
template <typename T>
struct DeserializeArrayImpl
{
    //! Deserialization functor.
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
struct DeserializeArrayImpl<std::array<uint8_t, N>>
{
    //! Deserialization functor.
    std::array<uint8_t, N> operator ()(uint8_t const * & in, size_t & size)
    {
        if (size < N)
            throw DeserializationError();
        std::array<uint8_t, N> a;
        std::copy(in, in + N, a.data());
        in   += N;
        size -= N;
        return a;
    }
};

template <typename T>
T deserialize(uint8_t const * & in, size_t & size)
{
    return T(in, size);
}

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

template <typename T>
T deserializeArray(uint8_t const * & in, size_t & size)
{
    DeserializeArrayImpl<T> impl;
    return impl(in, size);
}

template <typename T, size_t N>
std::vector<std::array<T, N>> deserializeVector(size_t n, uint8_t const * & in, size_t & size)
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

/******************************************************************************************************************/
/*                                 V A R I A B L E   L E N G T H   I N T E G E R                                  */
/******************************************************************************************************************/

//! A compressed 64-bit value.
//!
//! This value is primarily used in serialization of arrays to indicate the number of elements in an array. In the
//! reference code, it is known as CompactSize.
//!
//! @sa     VarArray

class VASize : public Serializable
{
public:
    // Constructor
    //!
    //! @param  v       The value
    explicit VASize(uint64_t v) : value_(v) {}

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    VASize(uint8_t const * & in, size_t & size);

    //! @name Overrides Serializable
    //!@{
    virtual void           serialize(std::vector<uint8_t> & out) const override;
    virtual nlohmann::json toJson() const override;

    //!@}

    //! Returns the value
    uint64_t value() const { return value_; }

private:
    uint64_t value_;
};

/******************************************************************************************************************/
/*                                          V A R I A B L E   A R R A Y                                           */
/******************************************************************************************************************/

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

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    VarArray(uint8_t const * & in, size_t & size)
    {
        VASize arraySize(in, size);
        data_ = P2p::deserializeVector<T>(arraySize.value(), in, size);
    }

    //! @name Overrides Serializable
    //!@{
    virtual void serialize(std::vector<uint8_t> & out) const override
    {
        P2p::serialize(VASize(data_.size()), out);
        P2p::serialize(data_, out);
    }

    virtual nlohmann::json toJson() const override
    {
        return P2p::toJson(data_);
    }

    //!@}

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
class VarArray<std::array<T, N>> : public Serializable
{
public:
    // Constructor
    VarArray() {}

    // Constructor
    //!
    //! @param  v       The elements to be contained in the array
    explicit VarArray(std::vector<std::array<T, N>> const & v) : data_(v) {}

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    VarArray(uint8_t const * & in, size_t & size)
    {
        VASize arraySize(in, size);
        data_ = P2p::deserializeVector<T, N>(arraySize.value(), in, size);
    }

    //! @name Overrides Serializable
    //!@{
    virtual void serialize(std::vector<uint8_t> & out) const override
    {
        P2p::serialize(VASize(data_.size()), out);
        P2p::serialize(data_, out);
    }

    nlohmann::json toJson() const override
    {
        return P2p::toJson(data_);
    }

    //!@}

    //! Returns the elements contained in the array
    std::vector<std::array<T, N>> value() const { return data_; }

private:
    std::vector<std::array<T, N>> data_;
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

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    VarArray(uint8_t const * & in, size_t & size)
    {
        VASize arraySize(in, size);
        data_ = P2p::deserializeVector<uint8_t>(arraySize.value(), in, size);
    }

    //! @name Overrides Serializable
    //!@{
    virtual void serialize(std::vector<uint8_t> & out) const override
    {
        P2p::serialize(VASize(data_.size()), out);
        P2p::serialize(data_, out);
    }

    nlohmann::json toJson() const override
    {
        return Utility::toHex(data_);
    }

    //!@}

    //! Returns the bytes contained in the array
    std::vector<uint8_t> value() const { return data_; }

private:
    std::vector<uint8_t> data_;
};

/******************************************************************************************************************/
/*                                      V A R I A B L E   B I T   A R R A Y                                       */
/******************************************************************************************************************/

//! An array of bits.
//!
//! This is primarily used for serialization of an array of bits along with the number of bits in the array.
//!
//! @sa VASize, VarArray

class BitArray : public P2p::Serializable
{
public:

    // Constructor
    BitArray() {}

    // Constructor
    //!
    //! @param  size    Size of the array in bits
    BitArray(size_t size)
        : bits_((size + 7) / 8, 0)
    {
    }

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    BitArray(uint8_t const * & in, size_t & size);

    //! Returns the value of the bit at the given index
    bool get(size_t index) const;

    //! Sets the value of the bit at the given index.
    //!
    //! @param  index       Which bit to set
    //! @param  value       Value to set the bit to (defaults to true)
    void set(size_t index, bool value = true);

    //! Sets the value of the bit at the given index to false.
    //!
    //! @param  index       Which bit to reset
    void reset(size_t index) { set(index, false); }

    //! Flips the value of the bit at the given index.
    //!
    //! @param  index       Which bit to flip
    void flip(size_t index) { set(index, !get(index)); }

    //! Returns the value of the bit at the given index
    //!
    //! @param  index       Which bit to get
    bool operator [](size_t index) const { return get(index); }

    //! @name Overrides Serializable
    //!@{
    virtual void           serialize(std::vector<uint8_t> & out) const override;
    virtual nlohmann::json toJson() const override;

    //!@}

private:

    std::vector<uint8_t> bits_;
};

/******************************************************************************************************************/
/*                                  V A R I A B L E   L E N G T H   S T R I N G                                   */
/******************************************************************************************************************/

//! A variable-length string.
//!
//! This is primarily used for serialization of a string along with the number of characters in the string.
//!
//! @sa VASize

class VarString : public Serializable
{
public:
    // Constructor
    VarString() {}

    // Constructor
    //!
    //! @param  s       The bytes to be contained in the array
    VarString(std::string const & s) : string_(s) {}

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    VarString(uint8_t const * & in, size_t & size)
    {
        VASize stringSize(in, size);
        string_ = P2p::deserializeString(stringSize.value(), in, size);
    }

    //! @name Overrides Serializable
    //!@{
    virtual void serialize(std::vector<uint8_t> & out) const override
    {
        P2p::serialize(VASize(string_.size()), out);
        P2p::serialize(string_, out);
    }

    virtual nlohmann::json toJson() const override
    {
        return string_;
    }

    //!@}

    //! Returns the bytes contained in the array
    std::string value() const { return string_; }

private:
    std::string string_;
};

//!@}
} // namespace Utility
