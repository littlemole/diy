cmake_minimum_required(VERSION 2.8.9)
project(diycpp)
include(CTest)

IF (WIN32)
find_package(gtest REQUIRED)
ENDIF ()


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)
include_directories(${GTEST_INCLUDE_DIR})

file(GLOB HEADERS "include/diycpp/*.h")

add_library(diycpp INTERFACE)
target_include_directories(diycpp INTERFACE include/)

add_executable(basic_test t/test.cpp)
add_executable(raw_test t/raw.cpp)

IF (UNIX)

target_link_libraries(basic_test gtest diycpp pthread)
target_link_libraries(raw_test gtest diycpp pthread)

ELSEIF (WIN32)

target_link_libraries(basic_test ${GTEST_LIBRARY} diycpp Ws2_32 Rpcrt4 )
target_link_libraries(raw_test ${GTEST_LIBRARY} diycpp Ws2_32 Rpcrt4 )

ENDIF ()

add_test(NAME BasicTest COMMAND basic_test)
add_test(NAME RawTest COMMAND raw_test)
 
install(FILES ${HEADERS} DESTINATION include/diycpp)

install(FILES diycpp.pc DESTINATION lib/pkgconfig)
