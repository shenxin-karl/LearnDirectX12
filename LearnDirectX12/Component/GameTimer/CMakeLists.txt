cmake_minimum_required(VERSION 3.8)	
set(CMAKE_CXX_STANDARD 20)
project(GameTimerTest)

add_executable(GameTimerTest "test.cpp" "GameTimer.h" "GameTimer.cpp")