cmake_minimum_required(VERSION 3.8)	
project(D3D)

# 开启多线程编译 和 使用 c++latest 版本
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /EHsc /std:c++latest")
if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.12.25835)
	set(CMAKE_CXX20_STANDARD_COMPILE_OPTION "-std:c++latest")
	set(CMAKE_CXX20_EXTENSION_COMPILE_OPTION "-std:c++latest")
endif()

file(GLOB_RECURSE HEADER_FILES *.h *.hpp *.ini)
file(GLOB_RECURSE SOURCE_FILES *.cpp *.c)
file(GLOB_RECURSE SHADER_FILES *.hlsl)
set(AllFile ${HEADER_FILES} ${SOURCE_FILES} ${SHADER_FILES})

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

add_library(D3D STATIC ${AllFile})
set_target_properties(D3D PROPERTIES FOLDER "Component")
set_property(SOURCE ${SHADER_FILES} PROPERTY VS_TOOL_OVERRIDE "shader")	# 设置 hlsl 为 shader 资源文件

if (MSVC)
	## enable wstring_convert
	add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)	
endif(MSVC)

## 定义 库 shader 提供文件
add_definitions(-DD3D_HLSL_SHADER_PATH="${CMAKE_CURRENT_SOURCE_DIR}/HlslShader") 


target_include_directories(D3D PUBLIC 
	${PROJECT_COMPONENT_DIR}
	${PROJECT_LIBRAIES_DIR}
	${PROJECT_LIBRAIES_DIR}/assimp/include
	${PROJECT_BUILD_DIR}/Libraies/assimp/include/
	${PROJECT_LIBRAIES_DIR}/Dx12Renderer/
)



## 出来 d3d shader 资源
include(CmakeRC.cmake)
cmrc_add_resource_library(D3D-resources ALIAS D3D::rc
	NAMESPACE D3D			# 指定命名空间
	"HlslShader/BlurFilterCS.hlsl"
	"HlslShader/SobelFilterCS.hlsl"
	"HlslShader/FXAACS.hlsl"
	"HlslShader/SkyBox.hlsl"
	"HlslShader/PanoToCubeMapCS.hlsl"
	"HlslShader/ConvolutionIrradianceMapCS.hlsl"
	"HlslShader/PerFilterEnvMapCS.hlsl"
	"resources/BRDF_LUT.dds"
	"resources/DefaultAlbedoMap.dds"
	"resources/DefaultAOMap.dds"
	"resources/DefaultMetallicMap.dds"
	"resources/DefaultNormalMap.dds"
	"resources/DefaultRoughnessMap.dds"
	"config/ShaderDefine.lua"
)

set_target_properties(D3D-resources PROPERTIES FOLDER "CmrcResource")

target_link_libraries(${PROJECT_NAME} PRIVATE 
	GameTimer
	stb
	Dx12lib
	Script
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	D3D::rc
)
