cmake_minimum_required(VERSION 3.31)
project(Game)

set(CMAKE_CXX_STANDARD 23)

find_package(opengl_system)
find_package(glfw3)
find_package(glad)


add_executable(Game main.cpp
        ShaderTest.h)

target_link_libraries(Game
        opengl::opengl
        glfw
        glad::glad
)
