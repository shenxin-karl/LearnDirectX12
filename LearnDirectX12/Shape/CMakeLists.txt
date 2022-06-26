cmake_minimum_required(VERSION 3.8)	
project("Shape")

# 开启多线程编译 和 使用 c++latest 版本
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /std:c++latest")

file(GLOB_RECURSE HEADER_FILES *.h *.hpp *.ini)
file(GLOB_RECURSE SOURCE_FILES *.cpp *.c)
file(GLOB_RECURSE SHADER_FILES *.hlsl)

set(CppFile ${HEADER_FILES} ${SOURCE_FILES})
set(AllFile ${CppFile} ${SHADER_FILES})

foreach(fileItem ${AllFile})
	get_filename_component(PARENT_DIR "${fileItem}" DIRECTORY)
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
	string(REPLACE "/" "\\" GROUP "${GROUP}")
	set(GROUP "${GROUP}")
	source_group("${GROUP}" FILES "${fileItem}")
endforeach()

add_executable("Shape" ${AllFile})

set_property(SOURCE ${SHADER_FILES} PROPERTY VS_TOOL_OVERRIDE "shader")	# 设置 hlsl 为 shader 资源文件
# 设置 Shape 工作目录为 cmake 目录
set_property(TARGET "Shape" PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

target_include_directories(BoxApp PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${PROJECT_LIBRAIES_DIR}/assimp/include
	${PROJECT_BUILD_DIR}/Libraies/assimp/include/
)


target_link_libraries("Shape" PUBLIC 
	BaseApp
	Math
	Geometry
	D3D
	dx12lib
	assimp
)