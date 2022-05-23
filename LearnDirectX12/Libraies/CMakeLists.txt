cmake_minimum_required(VERSION 3.8)	
project("Libraies")

LIST(APPEND LibraiesSubDir "cmrc")
LIST(APPEND LibraiesSubDir "stb")
LIST(APPEND LibraiesSubDir "DirectXTex")
LIST(APPEND LibraiesSubDir "Imgui")

foreach(dirname ${LibraiesSubDir}) 
	add_subdirectory(${dirname})
endforeach()

