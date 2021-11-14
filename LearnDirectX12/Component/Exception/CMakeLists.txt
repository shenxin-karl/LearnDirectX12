cmake_minimum_required(VERSION 3.8)	
set(CMAKE_CXX_STANDARD 20)

project(Exception)
set(HEADER_FILE "ExceptionBase.h")
set(SOURCE_FILE "ExceptionBase.cpp")
set(ALL_FILE ${HEADER_FILE} ${SOURCE_FILE})
add_library(Exception STATIC ${ALL_FILE})

