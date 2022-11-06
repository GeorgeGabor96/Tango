@echo off

set common_flags=-MT -nologo -Gm- -GR- -EHa- -Oi -FC -Fmmapping.map -W4 -WX -wd4102 -wd4189 -wd4100 -wd4201 -wd4702 -D_CRT_SECURE_NO_WARNINGS
set debug=-Zi -DCHECKS -DMEMORY_MANAGE -DTIMING
set release=-O2 -Zi -DTIMING
set crazy=-O2

set mode=%debug%
if "%1" == "release" (set mode=%release%)
if "%1" == "crazy" (set mode=%crazy%)

call windows_cl_setup.bat x64

rmdir /s /q build
mkdir build

set include_roots=-I%cd%\src
set compiler_params=%include_roots% %mode%
set linker_params=-subsystem:console -opt:ref

pushd build
call cl %common_flags% ..\src\experiments\debugging.c %compiler_params% /link %linker_params%
popd