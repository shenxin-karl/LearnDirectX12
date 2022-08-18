cmake_minimum_required(VERSION 3.8)	
project("Libraies")

list(APPEND LibraiesSubDir "cmrc")
list(APPEND LibraiesSubDir "stb")
list(APPEND LibraiesSubDir "Imgui")
list(APPEND LibraiesSubDir "lua")
list(APPEND LibraiesSubDir "Dx12Renderer")

foreach(dirname ${LibraiesSubDir}) 
	add_subdirectory(${dirname})
endforeach()


set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
include(assimp)
