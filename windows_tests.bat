@echo off

set debug=/Od /Zi
set release=/O2 /Zi
set mode=%debug%
if "%1" == "realease" (set mode=%release%)

call windows_cl_setup.bat x64

rmdir /s /q build_tests
mkdir build_tests

set code_root=%cd%\src
set tests_root=%cd%\tests

pushd build_tests
call cl ..\tests\tests.c -I%code_root% -I%tests_root% %mode%
call tests.exe
popd