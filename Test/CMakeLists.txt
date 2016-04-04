file(GLOB CRYPTO_TESTS
    ${CMAKE_CURRENT_SOURCE_DIR}/crypto/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/crypto/*.h
)
source_group(crypto FILES ${CRYPTO_TESTS})

file(GLOB EQUITY_TESTS
    ${CMAKE_CURRENT_SOURCE_DIR}/equity/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/equity/*.h
)
source_group(equity FILES ${EQUITY_TESTS})

file(GLOB NETWORK_TESTS
    ${CMAKE_CURRENT_SOURCE_DIR}/network/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/network/*.h
)
source_group(network FILES ${NETWORK_TESTS})

file(GLOB VALIDATION_TESTS
    ${CMAKE_CURRENT_SOURCE_DIR}/validation/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/validation/*.h
)
source_group(validation FILES ${VALIDATION_TESTS})

file(GLOB UTILITY_TESTS
    ${CMAKE_CURRENT_SOURCE_DIR}/utility/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utility/*.h
)
source_group(utility FILES ${UTILITY_TESTS})

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

target_link_libraries(equity-test equity)
source_group(Sources FILES ${SOURCES})
