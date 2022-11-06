@echo off

set debug=/Zi -DCHECKS -DMEMORY_MANAGE -DTIMING
set release=/O2 /Zi -DTIMING
set crazy=/O2

set mode=%debug%
if "%1" == "release" (set mode=%release%)
if "%1" == "crazy" (set mode=%crazy%)

call windows_cl_setup.bat x64

rmdir /s /q build
mkdir build

set include_roots=-I%cd%\src
set compiler_params=%include_roots% %mode%

pushd build
call cl ..\src\experiments\debugging.c %compiler_params%
popd