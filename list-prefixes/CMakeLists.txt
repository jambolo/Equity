set(SOURCES
    main.cpp
)

add_executable(list-prefixes ${SOURCES})
target_link_libraries(list-prefixes equity nlohmann_json::nlohmann_json)

source_group(Sources FILES ${SOURCES})
