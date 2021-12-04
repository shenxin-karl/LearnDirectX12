@echo off
mkdir Build
cmake clean Build
set /P OPT=<cmake_option.txt
cmake -S %~dp0 %OPT% -B Build
pause