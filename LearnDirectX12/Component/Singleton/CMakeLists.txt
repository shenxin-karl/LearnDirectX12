cmake_minimum_required(VERSION 3.8)	
project(Singleton)

add_library(Singleton INTERFACE "Singleton.hpp")
set_target_properties("Singleton" PROPERTIES FOLDER "Component")

