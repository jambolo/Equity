#pragma once

//! @defgroup EquityGroup Bitcoin components library
//! Library of the components used by a Bitcoin node.

//! @brief Bitcoin components.
//!
//! This library provides the functionality and components of the Bitcoin protocol.
namespace Equity
{
//! @addtogroup EquityGroup
//!@{

//! Configuration settings singleton for the Equity module
struct Configuration
{
    //! Returns the configuration data instance
    static Configuration & get() { return instance_; }

private:

    static Configuration instance_;
};

//!@}
} // namespace Equity
