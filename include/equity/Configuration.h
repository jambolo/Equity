#pragma once

namespace Equity
{

//! Configuration settings singleton for the Equity module
struct Configuration
{
    //! Returns the configuration data instance
    static Configuration & get()  { return instance_; }

private:

    static Configuration instance_;
};

} // namespace Equity
