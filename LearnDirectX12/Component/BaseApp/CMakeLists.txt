cmake_minimum_required(VERSION 3.8)	
set(CMAKE_CXX_STANDARD 20)
project(BaseAppTest)

add_executable(BaseAppTest "BaseApp.h" "BaseApp.cpp")

target_include_directories(BaseAppTest PUBLIC
	../../
)