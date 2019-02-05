cmake_minimum_required (VERSION 2.8)
project (ArgonValidationService)

set (CMAKE_CXX_STANDARD 11)
set (ArgonValidationService_VERSION_MAJOR 0)
set (ArgonValidationService_VERSION_MINOR 1)
set (ArgonValidationService_VERSION_REVISION 0)

find_path(MTHD_INCLUDE_DIR
        NAMES
        microhttpd.h
        PATHS
        /opt/local
        /usr/local
        /usr
        ENV "PROGRAMFILES(X86)"
        ENV "HWLOC_ROOT"
        PATH_SUFFIXES
        include)

find_library(MHTD
        NAMES
        microhttpd
        libmicrohttpd.lib
        PATHS
        ENV "MICROHTTPD_ROOT"
        PATH_SUFFIXES
        lib)

if("${MHTD}" STREQUAL "MHTD-NOTFOUND")
    message(FATAL_ERROR "microhttpd library NOT found")
else()
    set(LIBS ${LIBS} ${MHTD})
    include_directories(AFTER ${MTHD_INCLUDE_DIR})
endif()

set(SOURCE
    main.cpp
    arguments.cpp
    argon2/blake2/blake2b.c
    argon2/argon2.cpp
    argon2/argon2profile_1_1_524288.c
    argon2/argon2profile_4_4_16384.c
    argon2/implementation.c
    argon2/base64.cpp)

configure_file (
        "${PROJECT_SOURCE_DIR}/config.h.in"
        "${PROJECT_BINARY_DIR}/config.h"
)

include_directories("${PROJECT_BINARY_DIR}")

add_executable(ArgonValidationService ${SOURCE})

target_compile_options(ArgonValidationService PRIVATE -march=native -mtune=native -O3)
target_link_libraries(ArgonValidationService ${LIBS})

