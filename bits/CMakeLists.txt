set(SOURCES
    main.cpp
)

include_directories(..)
add_executable(bits ${SOURCES})
source_group(Sources FILES ${SOURCES})

target_link_libraries(bits equity utility)

#find_package (OpenSSL)
#if (OPENSSL_FOUND)
#    include_directories(${OPENSSL_INCLUDE_DIR})
#    target_link_libraries (bits ${OPENSSL_LIBRARIES})
#endif (OPENSSL_FOUND)
