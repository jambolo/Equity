#include "Transaction.h"

#include "equity/Script.h"
#include "p2p/Serialize.h"
#include "utility/Endian.h"
#include "utility/Utility.h"

using namespace Equity;

Transaction::Input::Input(uint8_t const * & in, size_t & size)
{
    txid = Txid(in, size);
    outputIndex = P2p::deserialize<uint32_t>(in, size);
    script = P2p::VarArray<uint8_t>(in, size).value();
    sequence = P2p::deserialize<uint32_t>(in, size);
}

void Transaction::Input::serialize(std::vector<uint8_t> & out) const
{
    P2p::serialize(txid, out);
    P2p::serialize(outputIndex, out);
    P2p::VarArray<uint8_t>(script).serialize(out);
    P2p::serialize(sequence, out);
}

P2p::Serializable::cJSON_ptr Transaction::Input::toJson() const
{
    cJSON * object = cJSON_CreateObject();
    cJSON_AddItemToObject(object, "txid", txid.toJson()->release());
    cJSON_AddNumberToObject(object, "index", outputIndex);
    cJSON_AddItemToObject(object, "script", Script(script).toJson()->release());
    cJSON_AddNumberToObject(object, "sequence", sequence);
    return std::make_unique<cppJSON>(object);
}

Transaction::Output::Output(uint8_t const * & in, size_t & size)
{
    value = P2p::deserialize<uint64_t>(in, size);
    script = P2p::VarArray<uint8_t>(in, size).value();
}

void Transaction::Output::serialize(std::vector<uint8_t> & out) const
{
    P2p::serialize(value, out);
    P2p::serialize(P2p::VarArray<uint8_t>(script), out);
}

P2p::Serializable::cJSON_ptr Transaction::Output::toJson() const
{
    cJSON * object = cJSON_CreateObject();
    cJSON_AddNumberToObject(object, "value", (double)value);
    cJSON_AddItemToObject(object, "script", Script(script).toJson()->release());
    return std::make_unique<cppJSON>(object);
}

Transaction::Transaction(int version, InputList const & inputs, OutputList const & outputs, uint32_t lockTime)
    : version_(version)
    , inputs_(inputs)
    , outputs_(outputs)
    , lockTime_(lockTime)
    , valid_(true)
{
}

Transaction::Transaction(uint8_t const * & in, size_t & size)
    : valid_(false)
{
    version_ = Utility::littleEndian(P2p::deserialize<uint32_t>(in, size));
    // Only version 1 is valid now.
    if (version_ != 1)
        throw P2p::DeserializationError();

    inputs_ = P2p::VarArray<Input>(in, size).value();
    outputs_ = P2p::VarArray<Output>(in, size).value();
    lockTime_ = Utility::littleEndian(P2p::deserialize<uint32_t>(in, size));

    valid_ = true;
}

Transaction::Transaction(std::string const & json)
    : valid_(false)
{

}

void Transaction::serialize(std::vector<uint8_t> & out) const
{
    P2p::serialize(Utility::littleEndian(version_), out);
    P2p::serialize(P2p::VarArray<Input>(inputs_), out);
    P2p::serialize(P2p::VarArray<Output>(outputs_), out);
    P2p::serialize(Utility::littleEndian(lockTime_), out);
}

P2p::Serializable::cJSON_ptr Transaction::toJson() const
{
    cJSON * object = cJSON_CreateObject();
    cJSON_AddNumberToObject(object, "version", version_);
    cJSON_AddItemToObject(object, "inputs", P2p::toJson(inputs_)->release());
    cJSON_AddItemToObject(object, "outputs", P2p::toJson(outputs_)->release());
    cJSON_AddNumberToObject(object, "locktime", lockTime_);
    return std::make_unique<cppJSON>(object);
}
