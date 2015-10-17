set(SOURCES
	Test.cpp
    Crypto/TestCrypto.cpp
    Equity/TestEquity.cpp
	Equity/TestAddress.cpp
	Equity/TestBase58.cpp
	Equity/TestBase58Check.cpp
	Equity/TestPrivateKey.cpp
	Equity/TestPublicKey.cpp
    Network/TestNetwork.cpp
    Validate/TestValidate.cpp
    Utility/TestUtility.cpp
)

include_directories(..)

add_executable(equity_test ${SOURCES})

target_link_libraries(equity_test
    equity
    crypto
    utility
)

find_package (OpenSSL)
if (OPENSSL_FOUND)
  include_directories(${OPENSSL_INCLUDE_DIR})
  target_link_libraries (equity_test ${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)
