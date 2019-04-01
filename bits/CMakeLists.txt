set(SOURCES
    main.cpp
)

add_executable(bits ${SOURCES})
target_link_libraries(bits equity utility nlohmann_json::nlohmann_json)

source_group(Sources FILES ${SOURCES})
