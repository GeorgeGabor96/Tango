@echo off

set debug=/Od /Zi
set release=/O2 /Zi
set mode=%debug%
if "%1" == "realease" (set mode=%release%)

call windows_cl_setup.bat x64

rmdir /s /q build_tests
mkdir build_tests

set include_roots=-I%cd%\src -I%cd%\tests
set compiler_flags=-DCHECKS
set compiler_params=%include_roots% %mode% %compiler_flags%

pushd build_tests
call cl ..\tests\tests.c %compiler_params%
call tests.exe
popd