cmake_minimum_required(VERSION 3.8)	
set(CMAKE_CXX_STANDARD 20)
project(GameTimer)

file(GLOB_RECURSE SOURCE_FILES *.cpp *.c)
file(GLOB_RECURSE HEADER_FILES *.hpp *.h *.ini)
SET(AllFile ${SOURCE_FILES} ${HEADER_FILES})


SourceGroup(${AllFile})
add_library(GameTimer STATIC ${AllFile})

set_target_properties("GameTimer" PROPERTIES FOLDER "Component")

target_include_directories(GameTimer PUBLIC
	${PROJECT_COMPONENT_DIR}/
)


