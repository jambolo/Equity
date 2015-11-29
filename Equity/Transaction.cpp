#include "Transaction.h"

#include "equity/Script.h"
#include "utility/Serialize.h"
#include "utility/Utility.h"

using namespace Equity;

static char const TXID_LABEL[]      = "\"txid\":";
static char const INDEX_LABEL[]     = "\"index\":";
static char const SCRIPT_LABEL[]    = "\"script\":";
static char const SEQUENCE_LABEL[]  = "\"sequence\":";
static char const VALUE_LABEL[]     = "\"value\":";
static char const VERSION_LABEL[]   = "\"version\":";
static char const INPUTS_LABEL[]    = "\"inputs\":";
static char const OUTPUTS_LABEL[]   = "\"outputs\":";
static char const LOCKTIME_LABEL[]  = "\"locktime\":";

Transaction::Input::Input(uint8_t const * & in, size_t & size)
{
    txid = Txid(in, size);
    outputIndex = Utility::deserialize<uint32_t>(in, size);
    script = Utility::VarArray<uint8_t>(in, size).value();
    sequence = Utility::deserialize<uint32_t>(in, size);
}

void Transaction::Input::serialize(std::vector<uint8_t> & out) const
{
    txid.serialize(out);
    Utility::serialize(outputIndex, out);
    Utility::VarArray<uint8_t>(script).serialize(out);
    Utility::serialize(sequence, out);
}

std::string Transaction::Input::toJson() const
{
    std::string json;

    json += '{';
    json += TXID_LABEL + txid.toJson() + ',';
    json += INDEX_LABEL + std::to_string(outputIndex) + ',';
    json += SCRIPT_LABEL + Script(script).toJson() + ',';
    json += SEQUENCE_LABEL + std::to_string(sequence);
    json += '}';

    return json;
}

Transaction::Output::Output(uint8_t const * & in, size_t & size)
{
    value = Utility::deserialize<uint64_t>(in, size);
    script = Utility::VarArray<uint8_t>(in, size).value();
}

void Transaction::Output::serialize(std::vector<uint8_t> & out) const
{
    Utility::serialize(value, out);
    Utility::VarArray<uint8_t>(script).serialize(out);
}

std::string Transaction::Output::toJson() const
{
    std::string json;

    json += '{';
    json += VALUE_LABEL + std::to_string(value) + ',';
    json += SCRIPT_LABEL + Script(script).toJson();
    json += '}';

    return json;
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
    version_ = Utility::deserialize<uint32_t>(in, size);
    // Only version 1 is valid now.
    if (version_ != 1)
        throw Utility::DeserializationError();

    inputs_ = Utility::VarArray<Input>(in, size).value();
    outputs_ = Utility::VarArray<Output>(in, size).value();
    lockTime_ = Utility::deserialize<uint32_t>(in, size);

    valid_ = true;
}

Transaction::Transaction(std::string const & json)
    : valid_(false)
{

}

void Transaction::serialize(std::vector<uint8_t> & out) const
{
    Utility::serialize<uint32_t>(version_, out);
    Utility::VarArray<Input>(inputs_).serialize(out);
    Utility::VarArray<Output>(outputs_).serialize(out);
    Utility::serialize<uint32_t>(lockTime_, out);
}

std::string Transaction::toHex() const
{
    std::vector<uint8_t> raw;
    serialize(raw);
    return Utility::toHex(raw);
}

std::string Transaction::toJson() const
{
    std::string json;

    json += '{';
    json += VERSION_LABEL + std::to_string(version_) + ',';
    json += INPUTS_LABEL + Utility::toJson(inputs_) + ',';
    json += OUTPUTS_LABEL + Utility::toJson(outputs_) + ',';
    json += LOCKTIME_LABEL + std::to_string(lockTime_);
    json += '}';

    return json;
}
