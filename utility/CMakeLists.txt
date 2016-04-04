set(SOURCES
    Debug.h
    Endian.cpp
	Endian.h
    MerkleTree.cpp
    MerkleTree.h
    Serialize.cpp
    Serialize.h
    Utility.cpp
    Utility.h
)


include_directories(..)

find_package (OpenSSL)
if (OPENSSL_FOUND)
include_directories(${OPENSSL_INCLUDE_DIR})
endif (OPENSSL_FOUND)

add_library(utility ${SOURCES})
source_group(Sources FILES ${SOURCES})
