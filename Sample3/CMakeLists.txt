# Sub directory
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "-std=c++11 -g -Wall")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_definitions("-I/usr/local/Cellar/llvm/6.0.0/include/c++/v1/")
add_definitions("-I/usr/local/Cellar/glfw/3.2.1/include/")
add_definitions("-I/usr/local/Cellar/glew/2.1.0/include/")
add_definitions("-I/usr/local/Cellar/glm/0.9.9.0/include/")
add_definitions("-I$ENV{HOME}/Code/tmp/OpenGL/OpenCampus/")

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

file(GLOB SRC_FILES "sample3.cpp")
add_executable(sample3 ${SRC_FILES})
target_link_libraries(sample3 glfw glew ${OPENGL_LIBRARIES})
