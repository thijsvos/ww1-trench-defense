cmake_minimum_required(VERSION 3.15)
project(ww1td C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ── Find system GLFW ──────────────────────────────────────────────
find_package(glfw3 3.3 REQUIRED)

# ── GLAD (generated source, not a library) ────────────────────────
add_library(glad STATIC extern/glad/src/gl.c)
target_include_directories(glad PUBLIC extern/glad/include)

# ── stb_image (header-only, compiled in one TU) ──────────────────
add_library(stb STATIC src/stb_impl.c)
target_include_directories(stb PUBLIC extern/stb)

# ── miniaudio (header-only, compiled in one TU) ─────────────────
add_library(miniaudio STATIC src/miniaudio_impl.c)
target_include_directories(miniaudio PUBLIC extern/miniaudio)
if(APPLE)
    target_link_libraries(miniaudio PRIVATE
        "-framework AudioToolbox"
        "-framework CoreAudio"
        "-framework CoreFoundation"
    )
endif()
if(UNIX AND NOT APPLE)
    target_link_libraries(miniaudio PRIVATE dl m pthread)
endif()

# ── Main executable ───────────────────────────────────────────────
set(SOURCES
    src/main.c
    # core
    src/core/log.c
    src/core/memory.c
    src/core/window.c
    src/core/input.c
    src/core/td_time.c
    src/core/engine.c
    # math
    src/math/mat4.c
    # render
    src/render/shader.c
    src/render/camera.c
    src/render/renderer.c
    src/render/texture.c
    src/render/sprite_batch.c
    src/render/debug_draw.c
    src/render/atlas_gen.c
    # resource
    src/resource/resource_manager.c
    # game
    src/game/map.c
    src/game/path.c
    src/game/level.c
    src/game/enemy.c
    src/game/wave.c
    src/game/economy.c
    src/game/tower.c
    src/game/projectile.c
    src/game/particles.c
    src/game/game.c
    # audio
    src/audio/audio.c
    src/audio/synth.c
    src/audio/sound_defs.c
    # ui
    src/ui/ui.c
    src/ui/ui_widgets.c
    # states
    src/states/state.c
    src/states/state_menu.c
    src/states/state_difficulty.c
    src/states/state_levelselect.c
    src/states/state_briefing.c
    src/states/state_play.c
    src/states/state_pause.c
    src/states/state_gameover.c
    src/states/state_victory.c
)

add_executable(ww1td ${SOURCES})

target_include_directories(ww1td PRIVATE
    ${CMAKE_SOURCE_DIR}/extern/glad/include
    ${CMAKE_SOURCE_DIR}/extern/stb
    ${CMAKE_SOURCE_DIR}/extern/miniaudio
)

target_link_libraries(ww1td PRIVATE glad stb miniaudio glfw)

# macOS frameworks
if(APPLE)
    target_link_libraries(ww1td PRIVATE
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
    target_compile_definitions(ww1td PRIVATE GL_SILENCE_DEPRECATION)
endif()

# Linux
if(UNIX AND NOT APPLE)
    find_package(OpenGL REQUIRED)
    target_link_libraries(ww1td PRIVATE OpenGL::GL dl m pthread)
endif()

# Warnings
if(MSVC)
    target_compile_options(ww1td PRIVATE /W4)
else()
    target_compile_options(ww1td PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Copy assets to build directory
add_custom_command(TARGET ww1td POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:ww1td>/assets
)
