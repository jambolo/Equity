FILE(GLOB CRYPTO_TESTS
    ${CMAKE_CURRENT_SOURCE_DIR}/crypto/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/crypto/*.h
)
SOURCE_GROUP(crypto FILES ${CRYPTO_TESTS})

FILE(GLOB EQUITY_TESTS
    ${CMAKE_CURRENT_SOURCE_DIR}/equity/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/equity/*.h
)
SOURCE_GROUP(equity FILES ${EQUITY_TESTS})

FILE(GLOB NETWORK_TESTS
    ${CMAKE_CURRENT_SOURCE_DIR}/network/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/network/*.h
)
SOURCE_GROUP(network FILES ${NETWORK_TESTS})

FILE(GLOB VALIDATION_TESTS
    ${CMAKE_CURRENT_SOURCE_DIR}/validation/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/validation/*.h
)
SOURCE_GROUP(validation FILES ${VALIDATION_TESTS})

FILE(GLOB UTILITY_TESTS
    ${CMAKE_CURRENT_SOURCE_DIR}/utility/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utility/*.h
)
SOURCE_GROUP(utility FILES ${UTILITY_TESTS})

set(SOURCES
	Test.cpp
)

add_executable(equity-test
    ${SOURCES}
    ${CRYPTO_TESTS}
    ${EQUITY_TESTS}
    ${NETWORK_TESTS}
    ${VALIDATION_TESTS}
    ${UTILITY_TESTS}
)

include_directories(..)

target_link_libraries(equity-test
    crypto
    equity
	network
	validation
    utility
)
source_group(Sources FILES ${SOURCES})

find_package (OpenSSL)
if (OPENSSL_FOUND)
  include_directories(${OPENSSL_INCLUDE_DIR})
  target_link_libraries (equity-test ${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)
