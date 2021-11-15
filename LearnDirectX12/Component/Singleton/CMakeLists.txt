cmake_minimum_required(VERSION 3.8)	
set(CMAKE_CXX_STANDARD 20)
project(Singleton)

add_library(Singleton INTERFACE "Singleton.hpp")
set_target_properties("Singleton" PROPERTIES FOLDER "Component")

