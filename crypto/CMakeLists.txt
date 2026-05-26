set(SOURCES
    Ecc.cpp
    Ecc.h
    Pbkdf2.cpp
    Pbkdf2.h
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

target_link_libraries(crypto PUBLIC nlohmann_json::nlohmann_json)
target_include_directories(crypto PUBLIC ${INTERFACE_INCLUDE_PATH})

source_group(Sources FILES ${SOURCES})
