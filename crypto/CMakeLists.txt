set(SOURCES
    CppUtility.h
    #Ecc.cpp
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

set(INTERFACE_INCLUDE_PATH ${PROJECT_SOURCE_DIR})

add_library(crypto ${SOURCES})

target_link_libraries(crypto nlohmann_json::nlohmann_json)
#target_link_libraries(crypto OpenSSL::Crypto)
target_link_libraries(crypto C:/Users/John/Projects/3rdParty/wolfssl/lib/Debug/x64/wolfssl.lib ws2_32.lib)
target_include_directories(crypto PUBLIC ${INTERFACE_INCLUDE_PATH})
target_include_directories(crypto PRIVATE
    C:/Users/John/Projects/3rdParty/wolfssl/include
    C:/Users/John/Projects/3rdParty/wolfssl/include/IDE/WIN
)
target_compile_definitions(crypto PRIVATE
    -DWOLFSSL_USER_SETTINGS
    -DWOLFSSL_LIB
)

source_group(Sources FILES ${SOURCES})
