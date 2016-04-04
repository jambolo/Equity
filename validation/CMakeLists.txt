set(SOURCES
    ScriptEngine.cpp
    ScriptEngine.h
    Validator.cpp
    Validator.h
)

set(INTERFACE_INCLUDE_PATH ${PROJECT_SOURCE_DIR})

add_library(validation ${SOURCES})
target_link_libraries(validation crypto utility equity)
target_include_directories(validation PUBLIC ${INTERFACE_INCLUDE_PATH})

source_group(Sources FILES ${SOURCES})
