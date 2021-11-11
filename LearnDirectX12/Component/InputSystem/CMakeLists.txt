cmake_minimum_required(VERSION 3.8)	
set(CMAKE_CXX_STANDARD 20)
project(InputSystemTest)

add_executable(InputSystemTest "test.cpp"
	"InputSystem.h" "InputSystem.cpp"
	"Keyboard.h"	"Keyboard.cpp"
	"Mouse.h"		"Mouse.cpp"
	"Window.h"		"Window.cpp"
)