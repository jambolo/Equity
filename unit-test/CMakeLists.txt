set(VSCPPUNIT_DIR "C:/Program Files (x86)/Microsoft Visual Studio 12.0/VC/UnitTest")
set(VSCPPUNIT_INCLUDE_DIR ${VSCPPUNIT_DIR}/Include)
set(VSCPPUNIT_LIB_DIR ${VSCPPUNIT_DIR}/Lib)

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

#file(GLOB NETWORK_TESTS
#    ${CMAKE_CURRENT_SOURCE_DIR}/network/*.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/network/*.h
#)
#source_group(network FILES ${NETWORK_TESTS})
#
#file(GLOB P2P_TESTS
#    ${CMAKE_CURRENT_SOURCE_DIR}/utility/*.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/utility/*.h
#)
#source_group(p2p FILES ${P2P_TESTS})
#
#file(GLOB UTILITY_TESTS
#    ${CMAKE_CURRENT_SOURCE_DIR}/utility/*.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/utility/*.h
#)
#source_group(utility FILES ${UTILITY_TESTS})

set(SOURCES
    shared.cpp
    shared.h
    targetver.h
    ${CRYPTO_TESTS}
    ${EQUITY_TESTS}
    ${NETWORK_TESTS}
    ${P2P_TESTS}
    ${UTILITY_TESTS}
)

set(INCLUDE_PATHS
    ${VSCPPUNIT_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}
)

link_directories(${VSCPPUNIT_LIB_DIR})

add_library(unit-test SHARED ${SOURCES})

target_include_directories(unit-test PRIVATE ${INCLUDE_PATHS})
target_link_libraries(unit-test equity crypto utility p2p )
