@echo off
if not exist vcpkg (
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
cmd /C bootstrap-vcpkg.bat
cd ..
)




if not exist "out" mkdir out
cd out
if not exist "build" mkdir build
cd build
if not exist "x64-Debug" mkdir "x64-Debug"
if not exist "x64-Release" mkdir "x64-Release"
cd ..
cd ..


set CMAKE_TOOLCHAIN_FILE=%~dp0vcpkg\scripts\buildsystems\vcpkg.cmake
echo CMAKE_TOOLCHAIN_FILE=%CMAKE_TOOLCHAIN_FILE%

cmd /C win\build.bat

