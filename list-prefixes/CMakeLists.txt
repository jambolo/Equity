set(SOURCES
    main.cpp
)

include_directories(..)
add_executable(list-prefixes ${SOURCES})
target_link_libraries(list-prefixes equity crypto)

find_package (OpenSSL)
if (OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    target_link_libraries (list-prefixes ${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)
