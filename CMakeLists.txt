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

file(GLOB HEADERS "include/diycpp/*.h")

add_library(diycpp INTERFACE)

add_executable(Tests t/test.cpp)

IF (UNIX)

target_link_libraries(Tests gtest diycpp pthread)

ELSEIF (WIN32)

target_include_directories(diycpp INTERFACE include/)
target_link_libraries(Tests ${GTEST_LIBRARY} diycpp Ws2_32 Rpcrt4 )


ENDIF ()

add_test(NAME AllTests COMMAND Tests)

#file(COPY pem DESTINATION .)
 
install(FILES ${HEADERS} DESTINATION include/diycpp)

install(FILES diycpp.pc DESTINATION lib/pkgconfig)
