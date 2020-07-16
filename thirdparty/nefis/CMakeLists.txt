# ##############################################################################
# CMake Build File for HMDF
#
# Written By: Zach Cobell
#
# ##############################################################################
#
# The CMake build system enable HMDF to be deployed and built in a cross
# platform environment.
#
# ##############################################################################

# ##############################################################################
# GENERAL OPTIONS
# ##############################################################################
cmake_minimum_required(VERSION 3.0.2)
project(
  NEFIS
  LANGUAGES C
  VERSION 0.0.1)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckFunctionExists)

# ##############################################################################
# Compiler flags
# ##############################################################################
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
mark_as_advanced(CLEAR CMAKE_C_FLAGS_RELEASE)
mark_as_advanced(CLEAR CMAKE_C_FLAGS_DEBUG)
mark_as_advanced(CLEAR CMAKE_C_COMPILER)
# ##############################################################################

# ##############################################################################
# NEFIS Library
# ##############################################################################
set(NEFIS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/c2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/df.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/er.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/f2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nefis_version.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/oc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wl-xdr.c)
add_library(nefis STATIC ${NEFIS_SOURCES})

target_compile_definitions(nefis PRIVATE _GNU_SOURCE)
target_compile_definitions(nefis PRIVATE STR_LEN_AT_END)
target_compile_definitions(nefis PRIVATE FTN_UNDERSCORE)
target_compile_definitions(nefis PRIVATE _FILE_OFFSET_BITS=64)
target_compile_definitions(nefis PRIVATE PTR${CMAKE_SIZEOF_VOID_P})

IF(WIN32)
    target_compiel_definitions(nefis PRIVATE W64)
ELSE()
    target_compile_definitions(nefis PRIVATE LINUX64)
    target_compile_definitions(nefis PRIVATE HAVE_CONFIG_H)
ENDIF()
target_include_directories(nefis PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ##############################################################################
