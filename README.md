# Equity
Bitcoin protocol library and related applications

Learning by doing. The plan is to implement Bitcoin from scratch. This is the library that will be used to build all the applications.

## Libraries

* **crypto**: Provides cryptography support, currently implemented using openssl.
* **equity**: Bitcoin protocol library
* **network**: Bitcoin network layer
* **peer**: P2P network layer
* **utility**: Miscellaneous functions
* **validation**: Validates transactions and blocks

## Applications/tests
* **bits**: Converts a target value in decimal compact form to hex, 256-bit, and difficulty.
* **equity-test**: Unit tests for the equity library
* **list-prefixes**: Lists Base5Check address ranges of all version codes.
* **view-transaction**: Displays a transaction in human-readable form
