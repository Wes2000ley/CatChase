cmake_minimum_required(VERSION 3.31)
project(Game)

set(CMAKE_CXX_STANDARD 23)

# External dependencies
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_package(cglm REQUIRED)
find_package(glm REQUIRED)
find_package(freetype REQUIRED)
find_package(nlohmann_json REQUIRED)

# Optional: Static linking settings (Windows-specific)
set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")

# ✅ Game Executable
add_executable(Game
        main.cpp
        game.cpp game.h
        SHADER.cpp SHADER.h
        TEXTURE.cpp TEXTURE.h
        RESOURCE_MANAGER.cpp RESOURCE_MANAGER.h
        Dog.cpp Dog.h
        Enemy.cpp Enemy.h
        Enemies.h
        TileMap.cpp TileMap.h
        Collision.h
        TEXT_RENDERER.cpp TEXT_RENDERER.h
        Level.cpp Level.h
        LevelManager.cpp LevelManager.h
        PauseMenu.cpp PauseMenu.h
        EnemyRegistry.cpp EnemyRegistry.h
        DebugDraw.cpp DebugDraw.h

)

# ✅ Link libraries
target_link_libraries(Game
        OpenGL::GL
        glfw
        glad::glad
        cglm::cglm
        glm::glm
        Freetype::Freetype
        nlohmann_json::nlohmann_json

)

# ✅ Copy resources folder to build dir
add_custom_target(copy_resources ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        ${CMAKE_CURRENT_BINARY_DIR}/resources
        COMMENT "📁 Copying resources folder..."
)

add_dependencies(Game copy_resources)

# 🗜️ Compress game executable and resources into a zip
add_custom_command(
        TARGET Game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "📦 Compressing GamePackage.zip..."
        COMMAND ${CMAKE_COMMAND} -E tar cfv CatChase.zip --format=zip Game.exe resources
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "📦 Creating GamePackage.zip"
)
