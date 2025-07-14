set(SOURCES
    Debug.h
    Endian.cpp
    Endian.h
    Utility.cpp
    Utility.h
)

set(INTERFACE_INCLUDE_PATH ${PROJECT_SOURCE_DIR})

add_library(utility ${SOURCES})
target_include_directories(utility PUBLIC ${INTERFACE_INCLUDE_PATH})
target_link_libraries(utility crypto nlohmann_json::nlohmann_json)

source_group(Sources FILES ${SOURCES})
