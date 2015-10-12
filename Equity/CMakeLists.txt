set(SOURCES
	Address.cpp
	Address.h
	Base58.cpp
	Base58.h
	Base58Check.cpp
	Base58Check.h
	Input.cpp
	Input.h
	Output.cpp
	Output.h
	PrivateKey.cpp
	PrivateKey.h
	PublicKey.cpp
	PublicKey.h
	Script.h
	Transaction.cpp
	Transaction.h
	Txid.cpp
	Txid.h
	Utility.cpp
	Utility.h
	Wallet.cpp
	Wallet.h
)


include_directories(..)

find_package (OpenSSL)
if (OPENSSL_FOUND)
  include_directories(${OPENSSL_INCLUDE_DIR})
endif (OPENSSL_FOUND)

add_library(equity ${SOURCES})
