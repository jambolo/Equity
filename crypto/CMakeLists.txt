set(SOURCES
    CppUtility.h
    Ecc.cpp
    Ecc.h
    Random.cpp
    Random.h
    Ripemd.cpp
    Ripemd.h
    Sha1.cpp
    Sha1.h
    Sha256.cpp
    Sha256.h
)

find_package(OpenSSL)

set(INCLUDE_PATHS
    ${OPENSSL_INCLUDE_DIR}
)

set(INTERFACE_INCLUDE_PATH ${PROJECT_SOURCE_DIR})

add_library(crypto ${SOURCES})
target_link_libraries(crypto ${OPENSSL_LIBRARIES})
target_include_directories(crypto PUBLIC ${INTERFACE_INCLUDE_PATH} PRIVATE ${INCLUDE_PATHS})

source_group(Sources FILES ${SOURCES})
