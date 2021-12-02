cmake_minimum_required(VERSION 3.8)	
project(GameTimer)

file(GLOB_RECURSE SOURCE_FILES *.cpp *.c)
file(GLOB_RECURSE HEADER_FILES *.hpp *.h *.ini)
SET(AllFile ${SOURCE_FILES} ${HEADER_FILES})

foreach(fileItem ${AllFile})       
	# Get the directory of the source file
	get_filename_component(PARENT_DIR "${fileItem}" DIRECTORY)
	# Remove common directory prefix to make the group
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
	# Make sure we are using windows slashes
	string(REPLACE "/" "\\" GROUP "${GROUP}")
	# Group into "Source Files" and "Header Files"
	set(GROUP "${GROUP}")
	source_group("${GROUP}" FILES "${fileItem}")
endforeach()

add_library(GameTimer STATIC ${AllFile})

set_target_properties("GameTimer" PROPERTIES FOLDER "Component")

target_include_directories(GameTimer PUBLIC
	${PROJECT_COMPONENT_DIR}/
)


