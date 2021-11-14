cmake_minimum_required(VERSION 3.8)	
set(CMAKE_CXX_STANDARD 20)
project(BaseApp)

#SET(CMAKE_SYSTEM_VERSION, 19042)
add_executable(BaseApp "BaseApp.h" "BaseApp.cpp")

target_include_directories(BaseApp PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/../
)

target_link_libraries(BaseApp PUBLIC
	InputSystem
	GameTimer
	D3D
)	