@echo off

set debug=/Zi
set release=/O2 /Zi
set mode=%debug%
if "%1" == "release" (set mode=%release%)

call windows_cl_setup.bat x64

rmdir /s /q build
mkdir build

set code_root=%cd%\src

pushd build
call cl ..\src\containers\containers.c -I%code_root% %mode%
popd