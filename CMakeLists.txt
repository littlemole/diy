############################################
# preamble
############################################
cmake_minimum_required(VERSION 2.8.9)
cmake_policy(SET CMP0054 NEW)
project(diycpp)
include(CTest)

############################################
# c++ std
############################################

set(CMAKE_CXX_STANDARD 14)
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
    set(LINKERFLAGS "-stdlib=libc++ -fcoroutines-ts  -std=c++14")
    set(FLAGS "-stdlib=libc++ -fcoroutines-ts -D_RESUMABLE_FUNCTIONS_SUPPORTED")
endif()

############################################
# OS support (Linux/Windows)
############################################

IF (WIN32)

	set(OSLIBS "Ws2_32" "Rpcrt4")

	set(BUILDFLAGS "-await ${FLAGS}")

	set(LIBS ${GTEST_LIBRARIES} ${STDLIB}  )

 ELSEIF (UNIX)

    set(OSLIBS "pthread")

    set(BUILDFLAGS " ${FLAGS}")

    set(LIBS ${GTEST_LIBRARIES} ${STDLIB} )

ENDIF ()



############################################
# set linker and compiler flags
############################################

set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${LINKERFLAGS}" )
set(CMAKE_CXX_FLAGS  		"${CMAKE_CXX_FLAGS} ${BUILDFLAGS}")


############################################
# include directories
############################################

include_directories(include)
include_directories(${GTEST_INCLUDE_DIR} )

############################################
# sources
############################################

file(GLOB HEADERS "include/diy/*.h")

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

install(FILES ${HEADERS} DESTINATION include/diycpp)

install(FILES diycpp.pc DESTINATION lib/pkgconfig)

