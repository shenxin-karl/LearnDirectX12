cmake_minimum_required(VERSION 3.8)	
project("Libraies")

LIST(APPEND LibraiesSubDir "cmrc")
LIST(APPEND LibraiesSubDir "stb")
LIST(APPEND LibraiesSubDir "DirectXTex")
LIST(APPEND LibraiesSubDir "Imgui")
#LIST(APPEND LibraiesSubDir "assimp")

foreach(dirname ${LibraiesSubDir}) 
	add_subdirectory(${dirname})
endforeach()


set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
include(assimp)
