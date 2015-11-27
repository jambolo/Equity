#pragma once

namespace Equity
{

struct Configuration
{
    static Configuration & get()  { return instance_; }

private:

    static Configuration instance_;
};

} // namespace Equity
