cmake_minimum_required(VERSION 3.8)	
set(CMAKE_CXX_STANDARD 20)
project(InputSystem)

file(GLOB_RECURSE HEADER_FILES *.h *.hpp *.ini)
file(GLOB_RECURSE SOURCE_FILES *.c *.cpp)
SET(AllFile ${HEADER_FILES} ${SOURCE_FILES})

SourceGroup(${AllFile})
add_library("InputSystem" ${AllFile})
set_target_properties("InputSystem" PROPERTIES FOLDER "Component")

target_include_directories(InputSystem PUBLIC 
	${PROJECT_COMPONENT_DIR}/
)

target_link_libraries(InputSystem PUBLIC 
	GameTimer
)

