#pragma once

#include "equity/Script.h"
#include <vector>
#include <cstdint>
#include <stack>


namespace Validation
{


class ScriptEngine
{
public:

    bool run(Equity::Script const & script);

private:

    Equity::Script::Program::const_iterator findMatchingElse(Equity::Script::Program const & instructions,
                                                             Equity::Script::Program::const_iterator start) const;
    Equity::Script::Program::const_iterator findMatchingEndif(Equity::Script::Program const & instructions,
                                                              Equity::Script::Program::const_iterator start) const;
    Equity::Script::Program::const_iterator processBranch(Equity::Script::Program const & instructions,
                                                          Equity::Script::Program::const_iterator i,
                                                          bool condition);

    std::vector<std::vector<uint8_t> > mainStack_;
    std::vector<std::vector<uint8_t> > altStack_;
    std::stack<Equity::Script::Program::const_iterator> scopeStack_;
    size_t codeSeparator_;
};


} // namespace Validation

