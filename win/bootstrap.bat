@echo off
if not exist vcpkg (
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
cmd /C bootstrap-vcpkg.bat
cd ..
)


