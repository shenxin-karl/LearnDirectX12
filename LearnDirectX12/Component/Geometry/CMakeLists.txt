cmake_minimum_required(VERSION 3.8)	
project(Geometry)

# 开启多线程编译 和 使用 c++latest 版本
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /std:c++latest")

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

add_library(Geometry STATIC ${AllFile})

set_target_properties("Geometry" PROPERTIES FOLDER "Component")

target_include_directories(Geometry PUBLIC
	${PROJECT_COMPONENT_DIR}/
	${PROJECT_LIBRAIES_DIR}/assimp/include
	${PROJECT_BUILD_DIR}/Libraies/assimp/include/
	${PROJECT_LIBRAIES_DIR}/Dx12Renderer/
)

target_link_libraries(Geometry PUBLIC 
	Math
	assimp
)


