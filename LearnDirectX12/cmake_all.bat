@echo off
rd /s /q %~dp0Build
mkdir Build
set /P OPT=<cmake_option.txt
rem for /d %%i in (*) do (
rem 	if exist %%i/CMakeLists.txt (
rem 		mkdir %~dp0Build\%%i
rem 		cmake -S %~dp0%%i %OPT% -B Build/%%i
rem 	)
rem )

cmake -S %~dp0 %OPT% -B Build
pause