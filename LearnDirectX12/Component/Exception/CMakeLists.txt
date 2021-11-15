cmake_minimum_required(VERSION 3.8)	
set(CMAKE_CXX_STANDARD 20)

project(Exception)
set(HEADER_FILE "ExceptionBase.h")
set(SOURCE_FILE "ExceptionBase.cpp")
set(AllFile ${HEADER_FILE} ${SOURCE_FILE})

SourceGroup(${AllFile})
add_library("Exception" STATIC ${AllFile})

set_target_properties("Exception" PROPERTIES FOLDER "Component")