#include "Transaction.h"

#include "equity/Script.h"
#include "p2p/Serialize.h"
#include "utility/Endian.h"
#include "utility/Utility.h"

using namespace Equity;

Equity::Transaction::Input::Input(json const & json)
    : txid(json["txid"])
    , outputIndex(json["outputIndex"])
    , script(Utility::fromHex(json["script"].get<std::string>()))
    , sequence(json["sequence"])
{
}

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

json Transaction::Input::toJson() const
{
    return json::object(
    {
        { "txid", txid.toJson() },
        { "outputIndex", outputIndex },
        { "script", Script(script).toJson() },
        { "sequence", sequence }
    });
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

json Transaction::Output::toJson() const
{
    return json::object(
    {
        { "value", (double)value },
        { "script", Script(script).toJson() }
    });
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
    version_ = Utility::Endian::little(P2p::deserialize<uint32_t>(in, size));
    // Only version 1 is valid now.
    if (version_ != 1)
        throw P2p::DeserializationError();

    inputs_ = P2p::VarArray<Input>(in, size).value();
    outputs_ = P2p::VarArray<Output>(in, size).value();
    lockTime_ = Utility::Endian::little(P2p::deserialize<uint32_t>(in, size));

    valid_ = true;
}

Transaction::Transaction(std::string const & json)
    : valid_(false)
{
}

void Transaction::serialize(std::vector<uint8_t> & out) const
{
    P2p::serialize(Utility::Endian::little(version_), out);
    P2p::serialize(P2p::VarArray<Input>(inputs_), out);
    P2p::serialize(P2p::VarArray<Output>(outputs_), out);
    P2p::serialize(Utility::Endian::little(lockTime_), out);
}

json Transaction::toJson() const
{
    return json::object(
    {
        { "version", version_ },
        { "inputs", P2p::toJson(inputs_) },
        { "outputs", P2p::toJson(outputs_) },
        { "locktime", lockTime_ }
    });
}
