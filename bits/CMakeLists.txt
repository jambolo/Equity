set(SOURCES
    main.cpp
)

add_executable(bits ${SOURCES})
target_link_libraries(bits equity utility)

source_group(Sources FILES ${SOURCES})
