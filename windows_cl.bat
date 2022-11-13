@echo off

set common_flags=-MT -nologo -Gm- -GR- -EHa- -Oi -FC -Fmmapping.map -W4 -WX -wd4102 -wd4189 -wd4100 -wd4201 -wd4702 -D_CRT_SECURE_NO_WARNINGS
set debug_flags=-Zi -DCHECKS -DMEMORY_MANAGE -DTIMING
set release_flags=-O2 -Zi -DTIMING
set crazy_flags=-O2

set mode="debug"
if "%1" == "release" (set mode="release")
if "%1" == "crazy" (set mode="crazy")

if %mode% == "debug" (
	set output_dir=build\debug
	set mode_flags=%debug_flags%
)
if %mode% == "release" (
	set output_dir=build\release
	set mode_flags=%release_flags%
)
if %mode% == "crazy" (
	set output_dir=build\crazy
	set mode_flags=%crazy_flags%
)

rmdir /s /q %output_dir%
mkdir %output_dir%

set include_roots=-I%cd%\src
set compiler_params=%include_roots% %mode_flags%
set linker_params=-subsystem:console -opt:ref

call windows_cl_setup.bat x64

pushd %output_dir%
call cl %common_flags% ..\..\src\experiments\debugging.c %compiler_params% /link %linker_params%
popd