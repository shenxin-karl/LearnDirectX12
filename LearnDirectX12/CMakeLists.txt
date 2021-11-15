cmake_minimum_required(VERSION 3.8)	
set(CMAKE_CXX_STANDARD 20)

project(AllProject)

# 获取当前文件夹的子目录
function(GetCMakeSubDir result rootDir)
	file(GLOB_RECURSE allItem LIST_DIRECTORIES true ${rootDir}/*)
	string(LENGTH ${rootDir}/ rootDirLength)
	foreach (dir ${allItem})
		if (IS_DIRECTORY ${dir} AND EXISTS ${dir}/CMakeLists.txt)
			string(SUBSTRING ${dir} ${rootDirLength} -1 subDir)
			LIST(APPEND allSubDir ${subDir})
		endif()
	endforeach()
	set(${result} ${allSubDir} PARENT_SCPOE)
endfunction()

# 获取相于顶层cmake的相对目录
set(TopDir ${CMAKE_CURRENT_SOURCE_DIR})
function(GetRelativeDir relativeDir)
	STRING(LENGTH ${TopDir}/ strLength)
	STRING(SUBSTRING ${CMAKE_CURRENT_SOURCE_DIR} ${strLength} -1 dir)
	set(${relativeDir} ${dir} PARENT_SCOPE)
endfunction()

# 文件按文件夹分类	
macro(SourceGroup allFile)
	foreach(fileItem ${allFile})    			 
		get_filename_component(PARENT_DIR "${fileItem}" DIRECTORY)
		string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
		string(REPLACE "/" "\\" GROUP "${GROUP}")
		if ("${fileItem}" MATCHES ".*\\.cpp")
			set(GROUP "Source Files${GROUP}")
		elseif("${fileItem}" MATCHES ".*\\.h")
			set(GROUP "Header Files${GROUP}")
		endif()
		source_group("${GROUP}" FILES "${fileItem}")
	endforeach()
endmacro()


LIST(APPEND allSubDir "Component")
#LIST(APPEND allSubDir "BoxApp")

foreach(dirname ${allSubDir})
	add_subdirectory(${dirname})
endforeach()

set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "Other")
