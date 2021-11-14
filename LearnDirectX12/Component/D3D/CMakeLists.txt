cmake_minimum_required(VERSION 3.8)	
project(D3D)

file(GLOB_RECURSE HEADER_FILES *.h *.hpp *.ini)
file(GLOB_RECURSE SOURCE_FILES *.cpp *.c)

add_library(D3D STATIC 
	${HEADER_FILES} ${SOURCE_FILES}
)

target_include_directories(D3D PUBLIC
	${PROJECT_COMPONENT_DIR}
)

target_link_libraries(D3D PUBLIC 
	GameTimer
)