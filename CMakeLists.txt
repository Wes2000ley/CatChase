cmake_minimum_required(VERSION 3.31)
project(Game)

set(CMAKE_CXX_STANDARD 23)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_package(cglm)
find_package(glm)
find_package(freetype)




add_executable(Game
        main.cpp
        game.cpp
        game.h
        SHADER.cpp
        SHADER.h
        TEXTURE.cpp
        TEXTURE.h
        RESOURCE_MANAGER.cpp
        RESOURCE_MANAGER.h
        Dog.cpp
        Dog.h
        Enemy.cpp
        Enemy.h
        Enemies.h
        TileMap.cpp
        TileMap.h
        Collision.h
        TEXT_RENDERER.cpp
        TEXT_RENDERER.h
        Level.cpp
        Level.h
        LevelManager.cpp
        LevelManager.h
)

target_link_libraries(Game
        OpenGL::GL
        glfw
        glad::glad
        cglm::cglm
        glm::glm
        Freetype::Freetype
)

# ✅ Copy resources folder to the build directory
add_custom_command(TARGET Game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        ${CMAKE_CURRENT_BINARY_DIR}/resources
)
