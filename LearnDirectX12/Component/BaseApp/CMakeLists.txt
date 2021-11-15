cmake_minimum_required(VERSION 3.8)	
set(CMAKE_CXX_STANDARD 20)
project(BaseApp)

file(GLOB_RECURSE SOURCE_FILES *.cpp *.c)
file(GLOB_RECURSE HEADER_FILES *.hpp *.h *.ini)
set(AllFile ${SOURCE_FILES} ${HEADER_FILES})		

SourceGroup(${AllFile})
add_library(BaseApp ${AllFile})
set_target_properties("BaseApp" PROPERTIES FOLDER "Component")

target_include_directories(BaseApp PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/../
)

target_link_libraries(BaseApp PUBLIC
	InputSystem
	GameTimer
	D3D
)	