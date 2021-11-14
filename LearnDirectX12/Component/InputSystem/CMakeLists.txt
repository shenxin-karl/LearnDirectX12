cmake_minimum_required(VERSION 3.8)	
set(CMAKE_CXX_STANDARD 20)
project(InputSystem)


file(GLOB_RECURSE HEADER_FILES *.h *.hpp *.ini)
file(GLOB_RECURSE SOURCE_FILES *.c *.cpp)
SET(ALL_FILE ${HEADER_FILES} ${SOURCE_FILES})
add_library(InputSystem ${ALL_FILE})
target_include_directories(InputSystem PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../")
target_link_libraries(InputSystem PUBLIC GameTimer)

