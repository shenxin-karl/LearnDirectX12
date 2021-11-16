cmake_minimum_required(VERSION 3.8)	
project(BoxApp)

file(GLOB_RECURSE HEADER_FILES *.h *.hpp *.ini)
file(GLOB_RECURSE SOURCE_FILES *.cpp *.c)

set(AllFile ${HEADER_FILES} ${SOURCE_FILES})

foreach(fileItem ${AllFile})
	get_filename_component(PARENT_DIR "${fileItem}" DIRECTORY)
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
	string(REPLACE "/" "\\" GROUP "${GROUP}")
	set(GROUP "${GROUP}")
	source_group("${GROUP}" FILES "${fileItem}")
endforeach()

add_executable(BoxApp ${AllFile})

target_include_directories(BoxApp PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(BoxApp PUBLIC 
	BaseApp
)