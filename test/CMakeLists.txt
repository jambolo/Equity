cmake_minimum_required (VERSION 3.10)

set(GTEST_ROOT "" CACHE PATH "Location of GTest")
find_package(GTest REQUIRED)
include(GoogleTest)

file(GLOB CRYPTO_TESTS
    ${CMAKE_CURRENT_SOURCE_DIR}/crypto/*.cpp
)
source_group(crypto FILES ${CRYPTO_TESTS})

foreach(FILE ${CRYPTO_TESTS})
    get_filename_component(TEST ${FILE} NAME_WE)
    set(TEST_EXE "${PROJECT_NAME}-crypto-${TEST}")
    add_executable(${TEST_EXE} ${FILE})
    target_link_libraries(${TEST_EXE} PRIVATE utility crypto GTest::GTest GTest::Main nlohmann_json::nlohmann_json)
    gtest_discover_tests(${TEST_EXE})
    target_compile_features(${TEST_EXE} PRIVATE cxx_std_17)
    set_target_properties(${TEST_EXE} PROPERTIES CXX_EXTENSIONS OFF)
    target_include_directories(${TEST_EXE} PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include)
endforeach()

file(GLOB EQUITY_TESTS
    ${CMAKE_CURRENT_SOURCE_DIR}/equity/*.cpp
)
source_group(equity FILES ${EQUITY_TESTS})

foreach(FILE ${EQUITY_TESTS})
    get_filename_component(TEST ${FILE} NAME_WE)
    set(TEST_EXE "${PROJECT_NAME}-equity-${TEST}")
    add_executable(${TEST_EXE} ${FILE})
    target_link_libraries(${TEST_EXE} PRIVATE utility crypto equity GTest::GTest GTest::Main nlohmann_json::nlohmann_json)
    gtest_discover_tests(${TEST_EXE})
    target_compile_features(${TEST_EXE} PRIVATE cxx_std_17)
    set_target_properties(${TEST_EXE} PROPERTIES CXX_EXTENSIONS OFF)
    target_include_directories(${TEST_EXE} PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include)
endforeach()

file(GLOB NETWORK_TESTS
    ${CMAKE_CURRENT_SOURCE_DIR}/network/*.cpp
)
source_group(network FILES ${NETWORK_TESTS})

foreach(FILE ${NETWORK_TESTS})
    get_filename_component(TEST ${FILE} NAME_WE)
    set(TEST_EXE "${PROJECT_NAME}_network-${TEST}")
    add_executable(${TEST_EXE} ${FILE})
    target_link_libraries(${TEST_EXE} PRIVATE utility network GTest::GTest GTest::Main nlohmann_json::nlohmann_json)
    gtest_discover_tests(${TEST_EXE})
    target_compile_features(${TEST_EXE} PRIVATE cxx_std_17)
    set_target_properties(${TEST_EXE} PROPERTIES CXX_EXTENSIONS OFF)
    target_include_directories(${TEST_EXE} PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include)
endforeach()

file(GLOB P2P_TESTS
    ${CMAKE_CURRENT_SOURCE_DIR}/p2p/*.cpp
)
source_group(p2p FILES ${P2P_TESTS})

foreach(FILE ${P2P_TESTS})
    get_filename_component(TEST ${FILE} NAME_WE)
    set(TEST_EXE "${PROJECT_NAME}-p2p-${TEST}")
    add_executable(${TEST_EXE} ${FILE})
    target_link_libraries(${TEST_EXE} PRIVATE utility p2p GTest::GTest GTest::Main nlohmann_json::nlohmann_json)
    gtest_discover_tests(${TEST_EXE})
    target_compile_features(${TEST_EXE} PRIVATE cxx_std_17)
    set_target_properties(${TEST_EXE} PROPERTIES CXX_EXTENSIONS OFF)
    target_include_directories(${TEST_EXE} PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include)
endforeach()

file(GLOB UTILITY_TESTS
    ${CMAKE_CURRENT_SOURCE_DIR}/utility/*.cpp
)
source_group(utility FILES ${UTILITY_TESTS})

foreach(FILE ${UTILITY_TESTS})
    get_filename_component(TEST ${FILE} NAME_WE)
    set(TEST_EXE "${PROJECT_NAME}-utility-${TEST}")
    add_executable(${TEST_EXE} ${FILE})
    target_link_libraries(${TEST_EXE} PRIVATE utility GTest::GTest GTest::Main nlohmann_json::nlohmann_json)
    gtest_discover_tests(${TEST_EXE})
    target_compile_features(${TEST_EXE} PRIVATE cxx_std_17)
    set_target_properties(${TEST_EXE} PROPERTIES CXX_EXTENSIONS OFF)
    target_include_directories(${TEST_EXE} PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include)
endforeach()
