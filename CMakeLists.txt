cmake_minimum_required(VERSION 3.26)
project(CPT205_Assessment_1)
include_directories(include)
link_directories(lib)
set(CMAKE_CXX_STANDARD 17)
add_executable(CPT205_Assessment_1 assessment-1-oop.cpp)

target_link_libraries(CPT205_Assessment_1 freeglut.dll opengl32.dll glu32.dll)
