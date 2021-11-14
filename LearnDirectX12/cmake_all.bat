@echo off
rd /s /q %~dp0Build
mkdir Build
set /P OPT=<cmake_option.txt
for /d %%i in (*) do (
	if exist %%i/CMakeLists.txt (
		mkdir %~dp0Build\%%i
		cmake -S %~dp0%%i %OPT% -B Build/%%i
	)
)
pause