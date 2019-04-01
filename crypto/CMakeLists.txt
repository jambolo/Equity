set(SOURCES
    CppUtility.h
    Ecc.cpp
    Ecc.h
    Hmac.cpp
    Hmac.h
    Pbkdf2.cpp
    Pbkdf2.h
    Random.cpp
    Random.h
    Ripemd.cpp
    Ripemd.h
    Sha1.cpp
    Sha1.h
    Sha256.cpp
    Sha256.h
    Sha512.cpp
    Sha512.h

)

find_package(OpenSSL)

set(INCLUDE_PATHS
    ${OPENSSL_INCLUDE_DIR}
)

set(INTERFACE_INCLUDE_PATH ${PROJECT_SOURCE_DIR})

add_library(crypto ${SOURCES})
target_link_libraries(crypto ${OPENSSL_LIBRARIES}  nlohmann_json::nlohmann_json)
target_include_directories(crypto PUBLIC ${INTERFACE_INCLUDE_PATH} PRIVATE ${INCLUDE_PATHS})

source_group(Sources FILES ${SOURCES})
