set(SOURCES
	Ecdsa.cpp
	Ecdsa.h
	Ripemd.cpp
	Ripemd.h
	Sha256.cpp
	Sha256.h
)

find_package (OpenSSL)
if (OPENSSL_FOUND)
  include_directories(${OPENSSL_INCLUDE_DIR})
endif (OPENSSL_FOUND)

add_library(crypto ${SOURCES})
