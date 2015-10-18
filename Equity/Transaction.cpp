#include "Transaction.h"

#include "utility\Serialize.h"
#include "utility\Utility.h"

using namespace Equity;

static std::string const TXID_LABEL("\"txid\":");
static std::string const INDEX_LABEL("\"index\":");
static std::string const SCRIPT_LABEL("\"script\":");
static std::string const SEQUENCE_LABEL("\"sequence\":");
static std::string const VALUE_LABEL("\"value\":");
static std::string const VERSION_LABEL("\"version\":");
static std::string const INPUTS_LABEL("\"inputs\":");
static std::string const OUTPUTS_LABEL("\"outputs\":");
static std::string const LOCKTIME_LABEL("\"locktime\":");

std::string Transaction::Input::toJson() const
{
    std::string json;

    json += '{';
    json += TXID_LABEL + txid.toJson() + ',';
    json += INDEX_LABEL + std::to_string(outputIndex) + ',';
    json += SCRIPT_LABEL + Utility::toJson(script) + ',';
    json += SEQUENCE_LABEL + std::to_string(sequence);
    json += '}';

    return json;
}

std::string Transaction::Output::toJson() const
{
    std::string json;

    json += '{';
    json += VALUE_LABEL + std::to_string(value) + ',';
    json += SCRIPT_LABEL + Utility::toJson(script) + ',';
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

Transaction::Transaction(std::vector<uint8_t> const & v)
    : valid_(false)
{

}

Transaction::Transaction(std::string const & json)
    : valid_(false)
{

}

void Transaction::serialize(std::vector<uint8_t> & out) const
{
}

std::string Transaction::toHex() const
{
    std::vector<uint8_t> raw;
    serialize(raw);
    return Utility::vtox(raw);
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
