set(SOURCES
	Base58.cpp
	Base58.h
	Base58Check.cpp
	Base58Check.h
	BitcoinAddress.cpp
	BitcoinAddress.h
	Input.cpp
	Input.h
	Output.cpp
	Output.h
	PrivateKey.cpp
	PrivateKey.h
	PublicKey.cpp
	PublicKey.h
	Transaction.cpp
	Transaction.h
	Txid.cpp
	Txid.h
	Wallet.cpp
	Wallet.h
)


include_directories(..)

find_package (OpenSSL)
if (OPENSSL_FOUND)
  include_directories(${OPENSSL_INCLUDE_DIR})
endif (OPENSSL_FOUND)

add_library(equity ${SOURCES})
