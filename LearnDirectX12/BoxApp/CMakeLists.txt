cmake_minimum_required(VERSION 3.8)	
project(BoxApp)

option(USE_SOLUTION_FOLDERS "使用资源管理器文件夹" ON)
option(GROUP_BY_EXPLORER ON) # 开启分组

file(GLOB_RECURSE HEADER_FILES *.h *.hpp *.ini)
file(GLOB_RECURSE SOURCE_FILES *.cpp *.c)
file(GLOB_RECURSE SHADER_FILES *.hlsl)

set(CppFile ${HEADER_FILES} ${SOURCE_FILES})
set(AllFile ${CppFile} ${SHADER_FILES})

message("AllFile: ${AllFile}")

foreach(fileItem ${AllFile})
	message("fileItem: ${fileItem}")
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