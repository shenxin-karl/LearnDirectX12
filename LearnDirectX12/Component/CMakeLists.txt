cmake_minimum_required(VERSION 3.8)	
set(CMAKE_CXX_STANDARD 20)

project(Component)

add_subdirectory(BaseAPP)
add_subdirectory(GameTimer)
add_subdirectory(InputSystem)

#target_include_directories(Component PUBLIC
	#${PROJECT_SOURCE_DIR}/..
#)