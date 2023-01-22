@echo off

REM usage: <script> [target<x64>]

REM (george): quit early if cl is visible
where /q cl
IF %ERRORLEVEL% == 0 (EXIT /b)

SET VC_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community
call "%VC_PATH%\VC\Auxiliary\Build\vcvarsall.bat" %1
