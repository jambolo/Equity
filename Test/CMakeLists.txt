set(SOURCES
	Test.cpp
	TestAddress.cpp
	TestBase58.cpp
	TestBase58Check.cpp
	TestPrivateKey.cpp
	TestPublicKey.cpp
    TestUtility.cpp
)

include_directories(..)

add_executable(equity_test ${SOURCES})

target_link_libraries(equity_test
    equity
    crypto
    utility
)

find_package (OpenSSL)
if (OPENSSL_FOUND)
  include_directories(${OPENSSL_INCLUDE_DIR})
  target_link_libraries (equity_test ${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)
