set(SOURCES
    Message.cpp
    Message.h
    Peer.cpp
    Peer.h
    Serialize.cpp
    Serialize.h
)
source_group(Sources FILES ${SOURCES})
    
set(PRIVATE_INCLUDE_PATH
    .
)

set(PUBLIC_INCLUDE_PATH
    ${PROJECT_SOURCE_DIR}
)

add_library(p2p ${SOURCES})
if(WIN32)
    target_link_libraries(p2p PUBLIC utility crypto nlohmann_json::nlohmann_json ws2_32.lib)
else()
    target_link_libraries(p2p PUBLIC utility crypto nlohmann_json::nlohmann_json)
endif()
target_include_directories(p2p PUBLIC ${PUBLIC_INCLUDE_PATH} PRIVATE ${PRIVATE_INCLUDE_PATH})

