cmake_minimum_required(VERSION 3.8)	
project(stb)

file(GLOB_RECURSE HEADER_FILES *.h *.hpp *.ini)
set(AllFile ${HEADER_FILES})

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

add_library(${PROJECT_NAME} INTERFACE ${AllFile})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Libraies")

