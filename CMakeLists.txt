cmake_minimum_required(VERSION 2.8.9)
project(diycpp)
include(CTest)

IF (WIN32)
find_package(gtest REQUIRED)

find_library(GTESTD gtestd)

set( GTEST_LIBS debug     ${GTESTD}
                optimized ${GTEST_LIBRARY} )

ENDIF ()




set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

############################################
# macros
############################################

macro(AddCompilerFlags target flags)
    get_target_property(CF ${target} COMPILE_FLAGS)
    if(CF STREQUAL "CF-NOTFOUND")
        SET(CF "") # set to empty string
    else()
        SET(CF "${CF} ") # a space to cleanly separate from existing content
    endif()

    SET(CF "${CF} ${flags}" )
    set_target_properties(${target} PROPERTIES COMPILE_FLAGS ${CF} )
endmacro()


############################################
# clang support
############################################

set(LINKERFLAGS " ")
set(STDLIB "")
set(FLAGS " ")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(STDLIB "c++abi")
    set(LINKERFLAGS "-stdlib=libc++ -fcoroutines-ts  -std=c++14")
    set(FLAGS "-stdlib=libc++ -fcoroutines-ts -D_RESUMABLE_FUNCTIONS_SUPPORTED")
endif()

set( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${LINKERFLAGS}" )



include_directories(include)
include_directories(${GTEST_INCLUDE_DIR})

file(GLOB HEADERS "include/diycpp/*.h")

add_library(diycpp INTERFACE)
target_include_directories(diycpp INTERFACE include/)

add_executable(basic_test t/test.cpp)
add_executable(raw_test t/raw.cpp)

AddCompilerFlags(basic_test ${FLAGS})
AddCompilerFlags(raw_test ${FLAGS})

IF (UNIX)

target_link_libraries(basic_test gtest diycpp pthread ${STDLIB})
target_link_libraries(raw_test gtest diycpp pthread ${STDLIB})

ELSEIF (WIN32)

target_link_libraries(basic_test ${GTEST_LIB} diycpp Ws2_32 Rpcrt4 )
target_link_libraries(raw_test ${GTEST_LIB} diycpp Ws2_32 Rpcrt4 )

ENDIF ()

add_test(NAME BasicTest COMMAND basic_test)
add_test(NAME RawTest COMMAND raw_test)
 
install(FILES ${HEADERS} DESTINATION include/diycpp)

install(FILES diycpp.pc DESTINATION lib/pkgconfig)
