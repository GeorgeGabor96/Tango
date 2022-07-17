@echo off

set compiler="cl"

if %compiler% == "cl" (call windows_cl.bat) ELSE (call windows_gcc.bat)