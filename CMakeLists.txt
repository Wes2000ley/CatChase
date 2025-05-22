cmake_minimum_required(VERSION 3.31)
project(Game)

set(CMAKE_CXX_STANDARD 23)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_package(cglm)
find_package(glm)



add_executable(Game
        main.cpp
        ShaderTest.h
)

target_link_libraries(Game
        OpenGL::GL
        glfw
        glad::glad
        cglm::cglm
        glm::glm
)

# ✅ Copy resources folder to the build directory
add_custom_command(TARGET Game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        ${CMAKE_CURRENT_BINARY_DIR}/resources
)
