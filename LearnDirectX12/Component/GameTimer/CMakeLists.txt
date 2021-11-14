cmake_minimum_required(VERSION 3.8)	
set(CMAKE_CXX_STANDARD 20)
project(GameTimer)

SET(SOURCE_FILE "GameTimer.cpp")
SET(HEADER_FILE "GameTimer.h")
SET(ALL_FILE ${SOURCE_FILE} ${HEADER_FILE})
add_library(GameTimer STATIC ${ALL_FILE})

target_include_directories(GameTimer PUBLIC
	${PROJECT_SOURCE_DIR}/
)


