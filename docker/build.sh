#!/bin/bash
set -e

echo "**********************************"
echo "building $1 for $BACKEND with" 
echo "$CXX using $BUILDCHAIN"
echo "**********************************"

function cmake_build {

    MODE="$1"
    
    mkdir -p $MODE
    cd $MODE

    cmake .. -DCMAKE_CXX_COMPILER=$CXX -DCMAKE_BUILD_TYPE=$MODE -DWITH_TEST=$WITH_TEST
     
    make
    
    if [ "$WITH_TEST" == "Off" ]
    then
    	echo "skipping tests for $1 ..."
    else
        ctest
    fi
    
    make install  
    cd ..
}


cd /usr/local/src/$1

if [ "$BUILDCHAIN" == "make" ] 
then
    if [ "$WITH_TEST" != "Off" ]
    then    
        make clean
        make -e test
    fi
    make clean
    make -e install
else
    cmake_build "Debug"
    cmake_build "Release"
fi


