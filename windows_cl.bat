@echo off

set debug=/Zi
set release=/O2 /Zi
set mode=%debug%
if "%1" == "release" (set mode=%release%)

call windows_cl_setup.bat x64

rmdir /s /q build
mkdir build

set include_roots=-I%cd%\src
set compiler_flags=-DCHECKS -DMEMORY_MANAGE
set compiler_params=%include_roots% %mode% %compiler_flags%

pushd build
call cl ..\src\experiments\debugging.c %compiler_params%
popd