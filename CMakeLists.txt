cmake_minimum_required (VERSION 3.9.2)
project (autotester)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake")
message (STATUS "${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR}")
if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are forbidden.")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE MATCHES Release)
    message("\n\n*********************\n\nRELEASE Build Type\n\n*********************\n\n")
else()
    if(CMAKE_BUILD_TYPE MATCHES ANSWER_GEN)
    message("\n\n*********************\n\nANSWER_GEN ENABLED\n\n*********************\n\n")
    set(CMAKE_CXX_FLAGS_ANSWER_GEN "-DANSWER_GEN")
    endif()
    set(CMAKE_BUILD_TYPE "") # Messes up autotester if the build type is really set
    message("\n\n*********************\n\nDEBUG Build Type\n\n*********************\n\n")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-std=c++17 -Wreturn-type -Wall -Wextra ${CMAKE_CXX_FLAGS_ANSWER_GEN}")

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNO_ASSERT")

if (WIN32)
    SET(CMAKE_FIND_LIBRARY_PREFIXES "")
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib" ".dll")
    set(AUTOTESTER_ROOT "${CMAKE_CURRENT_LIST_DIR}/lib/autotester/win")
elseif (APPLE)
    set(AUTOTESTER_ROOT "${CMAKE_CURRENT_LIST_DIR}/lib/autotester/apple")
elseif (UNIX)
    set(AUTOTESTER_ROOT "${CMAKE_CURRENT_LIST_DIR}/lib/autotester/unix")
endif()

list(APPEND CMAKE_PREFIX_PATH "${AUTOTESTER_ROOT}")
find_package(Autotester REQUIRED)
include_directories("${CMAKE_CURRENT_LIST_DIR}/lib")#include catch.hpp
include_directories("${CMAKE_CURRENT_LIST_DIR}/src/unit_testing/src")#include utility classes for unit_testing
add_subdirectory(src/spa)
add_subdirectory(src/autotester)
#add_subdirectory(src/autotester_gui)
add_subdirectory(src/unit_testing)
add_subdirectory(src/integration_testing)
