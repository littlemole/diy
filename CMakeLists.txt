############################################
# preamble
############################################

cmake_minimum_required(VERSION 3.22)

project(diycpp)
include(CTest)
set(VERSION 0.0.12)

option(WITH_TEST "run tests (default:on)" ON)

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
# clang support
############################################

set(LINKERFLAGS "")
set(STDLIB "")

if(WIN32)
    set(FLAGS "/W3")
    find_package(GTest REQUIRED)

    set(OSLIBS "Ws2_32" "Rpcrt4")
    set(LIBS ${GTEST_LIBRARIES} ${STDLIB}  )

else()
    set(FLAGS "-Wall -Wextra")

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(STDLIB "c++abi")
        set(LINKERFLAGS "-stdlib=libc++ -fcoroutines-ts")
        set(FLAGS "-stdlib=libc++ -fcoroutines-ts")
    endif()

    set(OSLIBS "pthread")
    set(LIBS ${STDLIB} )

endif()


############################################
# set linker and compiler flags
############################################

set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${LINKERFLAGS}" )
set(CMAKE_CXX_FLAGS  	    "${CMAKE_CXX_FLAGS} ${FLAGS}")


############################################
# gtest & jsoncpp
############################################
if(UNIX AND WITH_TEST)

    include(FetchContent)

    # gtest
    FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly.
        URL https://github.com/google/googletest/archive/f8d7d77c06936315286eb55f8de22cd23c188571.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    #FetchContent_MakeAvailable(googletest)

    FetchContent_GetProperties(googletest)
    if(NOT googletest_POPULATED)
        FetchContent_Populate(googletest)
        add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()
    
	set( LIBS "gtest" "${LIBS}")

endif()



message("+++++++++++++++++")
message("BUILDFLAGS: ${FLAGS}")
message("LIBS: ${LIBS}")
message("LDFLAGS: ${LINKER_FLAGS}")
message("+++++++++++++++++")


############################################
# include directories
############################################

include_directories(include)
if(WIN32 OR (WITH_TEST) )
    include_directories(${GTEST_INCLUDE_DIR} )
endif()

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

if(WITH_TEST)

add_executable(basic_test t/test.cpp)
add_executable(raw_test t/raw.cpp)
add_executable(static_test t/test_static.cpp)
add_executable(iface_test t/interface.cpp)
add_executable(iface_static_test t/interface_static.cpp)
add_executable(value_test t/value.cpp)

############################################
# libs for tests
############################################

target_link_libraries(basic_test ${LIBS} ${OSLIBS} )
target_link_libraries(raw_test ${LIBS} ${OSLIBS} )
target_link_libraries(static_test ${LIBS} ${OSLIBS} )
target_link_libraries(iface_test ${LIBS} ${OSLIBS} )
target_link_libraries(iface_static_test ${LIBS} ${OSLIBS} )
target_link_libraries(value_test ${LIBS} ${OSLIBS} )

############################################
# run tests
############################################

add_test(NAME BasicTest COMMAND basic_test)
add_test(NAME RawTest COMMAND raw_test)
add_test(NAME StaticTest COMMAND static_test)
add_test(NAME IFaceTest COMMAND iface_test)
add_test(NAME IFaceStaticTest COMMAND iface_static_test)
add_test(NAME ValueTest COMMAND value_test)

endif()

############################################
# install rules
############################################

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



