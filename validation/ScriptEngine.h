#pragma once

#include "equity/Script.h"
#include <cstdint>
#include <stack>
#include <vector>

namespace Validation
{

class ScriptEngine
{
public:

    bool run(Equity::Script const & script);

private:

    Equity::Program::const_iterator findMatchingElse(Equity::Program const &         instructions,
                                                     Equity::Program::const_iterator start) const;
    Equity::Program::const_iterator findMatchingEndif(Equity::Program const &         instructions,
                                                      Equity::Program::const_iterator start) const;
    Equity::Program::const_iterator processBranch(Equity::Program const &         instructions,
                                                  Equity::Program::const_iterator i,
                                                  bool                            condition);

    std::vector<std::vector<uint8_t> > mainStack_;
    std::vector<std::vector<uint8_t> > altStack_;
    std::stack<Equity::Program::const_iterator> scopeStack_;
    size_t codeSeparator_;
};

} // namespace Validation
