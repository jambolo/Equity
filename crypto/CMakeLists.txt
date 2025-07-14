set(SOURCES
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

# Not using OpenSSL
#find_package(OpenSSL 3.1 REQUIRED)
#add_compile_definitions(-DEQUITY_USING_OPENSSL)
find_package(wolfssl REQUIRED)

set(INTERFACE_INCLUDE_PATH ${PROJECT_SOURCE_DIR})

add_library(crypto ${SOURCES})

target_link_libraries(crypto PUBLIC nlohmann_json::nlohmann_json wolfssl::wolfssl)
target_include_directories(crypto PUBLIC ${INTERFACE_INCLUDE_PATH})
target_include_directories(crypto PRIVATE wolfssl::wolfssl)

source_group(Sources FILES ${SOURCES})
