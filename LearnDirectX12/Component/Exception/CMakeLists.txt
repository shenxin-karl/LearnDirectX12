cmake_minimum_required(VERSION 3.8)	

# 开启多线程编译 和 使用 c++latest 版本
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /std:c++latest")

project(Exception)
set(HEADER_FILE "ExceptionBase.h")
set(SOURCE_FILE "ExceptionBase.cpp")
set(AllFile ${HEADER_FILE} ${SOURCE_FILE})

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

add_library("Exception" STATIC ${AllFile})

set_target_properties("Exception" PROPERTIES FOLDER "Component")