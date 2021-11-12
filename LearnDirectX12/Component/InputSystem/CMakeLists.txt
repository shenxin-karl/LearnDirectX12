cmake_minimum_required(VERSION 3.8)	
set(CMAKE_CXX_STANDARD 20)
project(InputSystem)


SET(HEADER_FILE 
	"InputSystem.h"	"Keyboard.h"	
	"Mouse.h"		"Window.h"
)

SET(SOURCE_FILE
	"InputSystem.cpp"	"Keyboard.cpp"
	"Mouse.cpp"			"Window.cpp"
)

SET(ALL_FILE ${HEADER_FILE} ${SOURCE_FILE})
add_library(InputSystemLibrary ${ALL_FILE})

target_include_directories(InputSystemLibrary PUBLIC
	${PROJECT_SOURCE_DIR}
)

if (ENABLE_TEST) 
	enable_testing()
endif()

add_executable(InputSystemTest "InputSystemTest.cpp" ${ALL_FILE})
add_test(InputSystemTest .)
