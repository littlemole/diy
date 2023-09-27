cd out\build\x64-Debug
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=%~dp0\..\vcpkg\scripts\buildsystems\vcpkg.cmake ../../..
cmake --build .

cd ..

cd x64-Release
#cmake -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_TOOLCHAIN_FILE=%~dp0\..\vcpkg\scripts\buildsystems\vcpkg.cmake ../../..
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%~dp0\..\vcpkg\scripts\buildsystems\vcpkg.cmake ../../..
cmake --build .

cd ..
cd ..
cd ..

