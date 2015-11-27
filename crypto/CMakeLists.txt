set(SOURCES
	Ecdsa.cpp
	Ecdsa.h
	Ripemd.cpp
	Ripemd.h
	Sha1.cpp
	Sha1.h
	Sha256.cpp
	Sha256.h
)

find_package (OpenSSL)
if (OPENSSL_FOUND)
  include_directories(${OPENSSL_INCLUDE_DIR})
endif (OPENSSL_FOUND)

include_directories(..)

add_library(crypto ${SOURCES})
source_group(Sources FILES ${SOURCES})
