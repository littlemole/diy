############################################
# preamble
############################################

cmake_minimum_required(VERSION 3.22)

project(diycpp)
include(CTest)
set(VERSION 0.0.12)


############################################
# c++ std
############################################

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


############################################
# debug mode produces libs with d suffix
############################################

if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX d)
endif()


############################################
# basic dependecies
############################################

find_package(GTest REQUIRED)


############################################
# clang support
############################################

set(LINKERFLAGS "")
set(STDLIB "")
set(FLAGS "")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(STDLIB "c++abi")
    set(LINKERFLAGS "-stdlib=libc++ -fcoroutines-ts")
    set(FLAGS "-stdlib=libc++ -fcoroutines-ts")
endif()

############################################
# OS support (Linux/Windows)
############################################

IF (WIN32)

    set(OSLIBS "Ws2_32" "Rpcrt4")
    set(BUILDFLAGS "${FLAGS}")
    set(LIBS ${GTEST_LIBRARIES} ${STDLIB}  )

ELSEIF (UNIX)

    set(OSLIBS "pthread")
    set(BUILDFLAGS " ${FLAGS}")
    set(LIBS ${GTEST_LIBRARIES} ${STDLIB} )

ENDIF()


############################################
# set linker and compiler flags
############################################

set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${LINKERFLAGS}" )
set(CMAKE_CXX_FLAGS  	    "${CMAKE_CXX_FLAGS} ${BUILDFLAGS}")


############################################
# include directories
############################################

include_directories(include)
#include_directories(${GTEST_INCLUDE_DIR} )

############################################
# sources
############################################

file(GLOB_RECURSE HEADERS "include/diycpp/*.h")

############################################
# lib target
############################################

add_library(diycpp INTERFACE)
target_include_directories(diycpp INTERFACE include/)

############################################
# tests
############################################

add_executable(basic_test t/test.cpp)
add_executable(raw_test t/raw.cpp)

############################################
# libs for tests
############################################

target_link_libraries(basic_test ${LIBS} ${OSLIBS} )
target_link_libraries(raw_test ${LIBS} ${OSLIBS} )

############################################
# run tests
############################################

add_test(NAME BasicTest COMMAND basic_test)
add_test(NAME RawTest COMMAND raw_test)
 
############################################
# install rules
############################################
message( "${HEADERS}")
install(FILES ${HEADERS} DESTINATION include/diycpp)
install(FILES diycpp.pc DESTINATION lib/pkgconfig)


############################################
# Create and install package configuration 
# and version files.
############################################

configure_file(
   ${diycpp_SOURCE_DIR}/diycpp-config.cmake.in
   ${diycpp_BINARY_DIR}/diycpp-config.cmake @ONLY)

configure_file(
   ${diycpp_SOURCE_DIR}/diycpp-config-version.cmake.in
   ${diycpp_BINARY_DIR}/diycpp-config-version.cmake @ONLY)

install(FILES ${diycpp_BINARY_DIR}/diycpp-config.cmake
         ${diycpp_BINARY_DIR}/diycpp-config-version.cmake
         DESTINATION lib/diycpp-${VERSION})



