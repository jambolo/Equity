cmake_minimum_required (VERSION 3.10)
project(Equity LANGUAGES CXX DESCRIPTION "Bitcoin Node and Protocol LIbrary")

option(BUILD_SHARED_LIBS "Build libraries as DLLs" FALSE)

#########################################################################
# Build                                                                 #
#########################################################################

# Common external libraries

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    find_package(nlohmann_json REQUIRED)
endif()

# Common compiler defines

add_definitions(
    -DNOMINMAX
    -DWIN32_LEAN_AND_MEAN
    -DVC_EXTRALEAN
    -D_CRT_SECURE_NO_WARNINGS
    -D_SECURE_SCL=0
    -D_SCL_SECURE_NO_WARNINGS
)

# Debug libraries are postfixed with "d"

if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX d)
endif()

# Libraries

add_subdirectory(crypto)            # Crypto support
add_subdirectory(equity)            # Bitcoin
add_subdirectory(network)           # Bitcoin network
add_subdirectory(p2p)               # P2P network
add_subdirectory(utility)           # Utility functions

# Applications and tests

add_subdirectory(bits)              # Converts difficulty and target values
add_subdirectory(list-prefixes)     # Lists address ranges for all prefix values
add_subdirectory(view-transaction)  # Display a transaction in human-readable form

#########################################################################
# Documentation                                                         #
#########################################################################

set(${PROJECT_NAME}_DOXYGEN_OUTPUT_DIRECTORY "" CACHE PATH "Doxygen output directory (empty to disable)")
if(${PROJECT_NAME}_DOXYGEN_OUTPUT_DIRECTORY)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_OUTPUT_DIRECTORY ${${PROJECT_NAME}_DOXYGEN_OUTPUT_DIRECTORY})
        configure_file(Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile)
        add_custom_target(doxygen ALL ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            COMMENT "Generating API documentation with Doxygen" VERBATIM
            SOURCES ${PROJECT_BINARY_DIR}/Doxyfile
        )
    endif(DOXYGEN_FOUND)
endif()

#########################################################################
# Testing                                                               #
#########################################################################

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
    message(STATUS "Testing is enabled. Turn on BUILD_TESTING to build tests.")
    if(BUILD_TESTING)
        add_subdirectory(test)
    endif()
endif()

#########################################################################
# Installation                                                          #
#########################################################################

#Not sure how to do this yet
#add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
#
#include(GNUInstallDirs)
#set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
#
#install(TARGETS ${PROJECT_NAME}
#    EXPORT ${PROJECT_NAME}-targets
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#)
#install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
#install(EXPORT ${PROJECT_NAME}-targets
#    FILE ${PROJECT_NAME}Targets.cmake
#    NAMESPACE ${PROJECT_NAME}::
#    DESTINATION ${INSTALL_CONFIGDIR}
#)
#
## Create a ConfigVersion.cmake file
#include(CMakePackageConfigHelpers)
#write_basic_package_version_file(
#    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
#    VERSION ${PROJECT_VERSION}
#    COMPATIBILITY AnyNewerVersion
#)
#
#configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
#    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
#    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
#)
#
## Install the config, configversion and custom find modules
#install(FILES
#    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
#    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
#    DESTINATION ${INSTALL_CONFIGDIR}
#)
#
## Exporting from the build tree
#export(EXPORT ${PROJECT_NAME}-targets FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake NAMESPACE ${PROJECT_NAME}::)
#
## Register package in user's package registry
#export(PACKAGE ${PROJECT_NAME})
