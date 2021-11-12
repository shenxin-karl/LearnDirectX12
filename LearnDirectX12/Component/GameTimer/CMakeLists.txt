cmake_minimum_required(VERSION 3.8)	
set(CMAKE_CXX_STANDARD 20)

SET(SOURCE_FILE "GameTimer.cpp")
SET(HEADER_FILE "GameTimer.h")
SET(ALL_FILE ${SOURCE_FILE} ${HEADER_FILE})
add_library(game_library STATIC ${ALL_FILE})

if (ENABLE_TEST)
	enable_testing()
endif()

add_executable(GameTimerTest "GameTimerTest.cpp" ${ALL_FILE})
add_test(GameTimerTest . COMMAND test)