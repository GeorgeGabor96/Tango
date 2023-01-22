@echo off
if not exist build mkdir build
pushd src 
gcc containers\containers.c -o ..\build\containers.exe -I.
popd
