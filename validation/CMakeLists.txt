set(SOURCES
    ScriptEngine.cpp
	ScriptEngine.h
    Validator.cpp
    Validator.h
)

include_directories(..)

add_library(validation ${SOURCES})
source_group(Sources FILES ${SOURCES})
