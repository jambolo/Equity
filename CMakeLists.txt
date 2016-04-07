cmake_minimum_required (VERSION 3.5)
project(Equity CXX)

#set(Equity_VERSION_MAJOR 0)
#set(Equity_VERSION_MINOR 1)
#configure_file("$(PROJECT_SOURCE_DIR)/Version.h.in" "$(PROJECT_BINARY_DIR)/Version.h")
#include_directories("$(PROJECT_BINARY_DIR)")

add_definitions(
    -DNOMINMAX
    -DWIN32_LEAN_AND_MEAN
    -DVC_EXTRALEAN
    -D_CRT_SECURE_NO_WARNINGS
    -D_SECURE_SCL=0
    -D_SCL_SECURE_NO_WARNINGS
)

option(BUILD_SHARED_LIBS "Build libraries as DLLs" FALSE)

# Libraries

add_subdirectory(crypto)            # Crypto support
add_subdirectory(equity)            # Bitcoin
add_subdirectory(network)           # Bitcoin network
add_subdirectory(p2p)               # P2P network
add_subdirectory(utility)           # Utility functions

# Applications and tests

add_subdirectory(bits)              # Converts difficulty and target values
add_subdirectory(list-prefixes)     # Lists address ranges for all prefix values
add_subdirectory(view-transaction)  # Display a transaction in human-readable form
