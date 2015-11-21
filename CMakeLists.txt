cmake_minimum_required (VERSION 3.4)
project(Equity)

add_definitions(
    -DNOMINMAX
    -DWIN32_LEAN_AND_MEAN
    -DVC_EXTRALEAN
    -D_CRT_SECURE_NO_WARNINGS
    -D_SECURE_SCL=0
    -D_SCL_SECURE_NO_WARNINGS
)

add_subdirectory(crypto)
add_subdirectory(equity)
add_subdirectory(list-prefixes)
add_subdirectory(network)
add_subdirectory(test)
add_subdirectory(utility)
add_subdirectory(validation)
add_subdirectory(view-transaction)

