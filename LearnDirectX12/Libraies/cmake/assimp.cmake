set(ASSIMP_INSTALL OFF)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory("assimp")

set_target_properties("assimp" PROPERTIES FOLDER "Libraies/assimp")
set_target_properties("UpdateAssimpLibsDebugSymbolsAndDLLs" PROPERTIES FOLDER "Libraies/assimp")
set_target_properties("assimp_cmd" PROPERTIES FOLDER "Libraies/assimp")
set_target_properties("zlibstatic" PROPERTIES FOLDER "Libraies/assimp")
 