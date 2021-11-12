cmake_minimum_required(VERSION 3.8)	
set(CMAKE_CXX_STANDARD 20)
project(Singleton)

add_library(Singleton INTERFACE "Singleton.hpp")

if (ENABLE_TEST))
	enable_testing()
endif()

add_test(SingletonTest .)