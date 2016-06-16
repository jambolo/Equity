set(SOURCES
    Address.cpp
    Base58.cpp
    Base58Check.cpp
    Block.cpp
    Configuration.cpp
    Instruction.cpp
    Mnemonic.cpp
    PrivateKey.cpp
    PublicKey.cpp
    Script.cpp
    ScriptEngine.cpp
    ScriptEngine.h
    Target.cpp
    Transaction.cpp
    Txid.cpp
    Wallet.cpp
    Validator.cpp
    Validator.h
)

set(HEADERS
    ${PROJECT_SOURCE_DIR}/include/equity/Address.h
    ${PROJECT_SOURCE_DIR}/include/equity/Base58.h
    ${PROJECT_SOURCE_DIR}/include/equity/Base58Check.h
    ${PROJECT_SOURCE_DIR}/include/equity/Block.h
    ${PROJECT_SOURCE_DIR}/include/equity/Configuration.h
    ${PROJECT_SOURCE_DIR}/include/equity/Instruction.h
    ${PROJECT_SOURCE_DIR}/include/equity/Mnemonic.h
    ${PROJECT_SOURCE_DIR}/include/equity/PrivateKey.h
    ${PROJECT_SOURCE_DIR}/include/equity/PublicKey.h
    ${PROJECT_SOURCE_DIR}/include/equity/Script.h
    ${PROJECT_SOURCE_DIR}/include/equity/Target.h
    ${PROJECT_SOURCE_DIR}/include/equity/Transaction.h
    ${PROJECT_SOURCE_DIR}/include/equity/Txid.h
    ${PROJECT_SOURCE_DIR}/include/equity/Wallet.h
)

find_package(OpenSSL)

set(INCLUDE_PATHS
    ${PROJECT_SOURCE_DIR}/include/equity
    ${OPENSSL_INCLUDE_DIR}
)

set(INTERFACE_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/include)

add_library(equity ${SOURCES} ${HEADERS})
target_include_directories(equity PUBLIC ${INTERFACE_INCLUDE_PATH} PRIVATE ${INCLUDE_PATHS})
target_link_libraries(equity crypto utility p2p ${OPENSSL_LIBRARIES})

source_group(Sources FILES ${SOURCES})
source_group(Headers FILES ${HEADERS})
