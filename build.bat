@echo off

set debug_flags=-DCHECKS -DMEMORY_MANAGE -DTIMING -D_DEBUG_
set release_flags=-O2 -DTIMING
set crazy_flags=-O2

set mode="debug"
if "%1" == "release" (set mode="release")
if "%1" == "crazy" (set mode="crazy")

if %mode% == "debug" (
	set output_dir=bin\debug
	set mode_flags=%debug_flags%
)
if %mode% == "release" (
	set output_dir=bin\release
	set mode_flags=%release_flags%
)
if %mode% == "crazy" (
	set output_dir=bin\crazy
	set mode_flags=%crazy_flags%
)

rmdir /s /q %output_dir%
mkdir %output_dir%

set compiler_flags=-g -Wvarargs -Wall -Werror -Wno-misleading-indentation -Wno-dangling-else -Wno-unused-function -Wno-unused-label -Wno-unused-variable
set include_flags=-I%cd%/src
set linker_flags=-luser32
set defines=-D_CRT_SECURE_NO_WARNINGS

pushd %output_dir%
clang ../../src/experiments/cathing_up.c %compiler_flags% -o debugging.exe %defines% %mode_flags% %include_flags% %linker_flags%

debugging.exe

popd

echo Build Finished
