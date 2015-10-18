set(SOURCES
    Validator.cpp
    Validator.h
)

include_directories(..)

add_library(validation ${SOURCES})
