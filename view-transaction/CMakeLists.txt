set(SOURCES
    main.cpp
)

add_executable(view-transaction ${SOURCES})
target_link_libraries(view-transaction equity nlohmann_json::nlohmann_json)

source_group(Sources FILES ${SOURCES})
