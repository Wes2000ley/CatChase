cmake_minimum_required(VERSION 3.31)
project(Game)

set(CMAKE_CXX_STANDARD 23)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_package(cglm)
find_package(glm)
find_package(freetype)
find_package(nlohmann_json)


set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")



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
        PauseMenu.cpp
        PauseMenu.h
)

target_link_libraries(Game
        OpenGL::GL
        glfw
        glad::glad
        cglm::cglm
        glm::glm
        Freetype::Freetype
        nlohmann_json::nlohmann_json
)
# ✅ Always copy resources folder to build dir
add_custom_target(copy_resources ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        ${CMAKE_CURRENT_BINARY_DIR}/resources
        COMMENT "📁 Copying resources folder..."
)

add_dependencies(Game copy_resources)
if (WIN32)
    set_target_properties(Game PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
