cmake_minimum_required(VERSION 3.31)
project(Game LANGUAGES CXX)

# ───────────────────────────────
# ✅ C++ Standard and Options
# ───────────────────────────────
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ───────────────────────────────
# ✅ Compiler Warnings and Optimization
# ───────────────────────────────
if(MSVC)
    add_compile_options(/W4 /permissive- /EHsc)
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /RTC1 /MDd")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /GL /DNDEBUG /MD")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/LTCG /INCREMENTAL:NO /OPT:REF /OPT:ICF")
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion)
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -ffast-math -fno-omit-frame-pointer -DNDEBUG")
endif()

# ───────────────────────────────
# ✅ Static linking (MinGW only)
# ───────────────────────────────
if(WIN32 AND NOT MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
endif()

# ───────────────────────────────
# ✅ Dependencies
# ───────────────────────────────
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_package(cglm REQUIRED)
find_package(glm REQUIRED)
find_package(Freetype REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(nuklear REQUIRED)

# ───────────────────────────────
# ✅ Sources
# ───────────────────────────────
file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/*.c"
        "${CMAKE_SOURCE_DIR}/src/*.h"
        "${CMAKE_SOURCE_DIR}/src/*.hpp"
)

add_executable(Game ${GAME_SOURCES})
target_include_directories(Game PRIVATE "${CMAKE_SOURCE_DIR}/src")

target_link_libraries(Game
        OpenGL::GL
        glfw
        glad::glad
        cglm::cglm
        glm::glm
        Freetype::Freetype
        nlohmann_json::nlohmann_json
        nuklear::nuklear
)

# ───────────────────────────────
# ✅ Copy resources
# ───────────────────────────────
add_custom_target(copy_resources ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/resources"
        "$<TARGET_FILE_DIR:Game>/resources"
        COMMENT "📁 Copying resources..."
)
add_dependencies(Game copy_resources)
# ───────────────────────────────
# ✅ Optional Self-Signing Step (Windows)
# ───────────────────────────────
if (WIN32 AND MSVC)
    set(SIGNTOOL_PATH "C:/Program Files (x86)/Windows Kits/10/bin/10.0.18362.0/x64/signtool.exe")

    add_custom_command(
            TARGET Game POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "🔏 Self-signing Game.exe with local certificate..."
            COMMAND powershell -NoLogo -NoProfile -Command ^
            "$cert = Get-ChildItem Cert:\\CurrentUser\\My | Where-Object { $_.Subject -eq 'CN=GameDevLocal' }; " ^
            "if ($cert) { & '${SIGNTOOL_PATH}' sign /n 'GameDevLocal' /fd SHA256 /tr http://timestamp.digicert.com /td SHA256 '$<TARGET_FILE:Game>'; } " ^
            "else { Write-Host '⚠️ No local signing certificate found (CN=GameDevLocal). Skipping.' }"
            COMMENT "🔏 Signing build for local trust"
    )
endif()

# ───────────────────────────────
# ✅ Windows-safe ZIP packaging (PowerShell)
# ───────────────────────────────
add_custom_command(
        TARGET Game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "📦 Packaging Game build into Game-Package.zip (PowerShell)..."
        # Create staging folder
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Game>/Package"
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:Game>" "$<TARGET_FILE_DIR:Game>/Package/"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "$<TARGET_FILE_DIR:Game>/resources"
        "$<TARGET_FILE_DIR:Game>/Package/resources"
        # Use PowerShell to zip (works everywhere on Win10/11)
        COMMAND powershell -NoLogo -NoProfile -Command ^
        "Compress-Archive -Path '$<TARGET_FILE_DIR:Game>\\Package\\*' -DestinationPath '$<TARGET_FILE_DIR:Game>\\CatChase.zip' -Force"
        COMMENT "📦 Creating Game-Package.zip via PowerShell"
)
