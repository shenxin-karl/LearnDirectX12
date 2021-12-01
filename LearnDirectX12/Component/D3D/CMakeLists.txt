cmake_minimum_required(VERSION 3.8)	
project(D3D)

file(GLOB_RECURSE HEADER_FILES *.h *.hpp *.ini)
file(GLOB_RECURSE SOURCE_FILES *.cpp *.c)
file(GLOB_RECURSE SHADER_FILES *.hlsl)
set(AllFile ${HEADER_FILES} ${SOURCE_FILES} ${SHADER_FILES})

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

add_library(D3D STATIC ${AllFile})
set_target_properties(D3D PROPERTIES FOLDER "Component")
set_property(SOURCE ${SHADER_FILES} PROPERTY VS_TOOL_OVERRIDE "shader")	# 设置 hlsl 为 shader 资源文件

target_include_directories(D3D PUBLIC 
	${PROJECT_COMPONENT_DIR}
)

target_link_libraries(D3D PUBLIC 
	GameTimer
	Exception
)