# Sub directory
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "-std=c++11 -g -Wall")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_definitions("-I/usr/local/Cellar/llvm/6.0.0/include/c++/v1/")
add_definitions("-I/usr/local/Cellar/glfw/3.2.1/include/")
add_definitions("-I/usr/local/Cellar/glew/2.1.0/include/")
add_definitions("-I/usr/local/Cellar/glm/0.9.9.3/include/")
add_definitions("-I$ENV{HOME}/Code/tmp/OpenGL/OpenCampus/")

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

file(GLOB SRC_FILES "sample2.cpp")
link_directories(/usr/local/lib)
add_executable(sample2 ${SRC_FILES})
target_link_libraries(sample2 glfw glew ${OPENGL_LIBRARIES})
