#!/bin/bash

# WASM Build Script for Raycaster Engine

set -e

# Set Emscripten cache directory and disable frozen cache
export EM_CACHE=~/.emscripten_cache
unset FROZEN_CACHE

echo "Building Raycaster for WebAssembly..."

# Create build directory
mkdir -p build-wasm
cd build-wasm

# Compile all source files
emcc -O3 \
    -I../include \
    -I../src/assets \
    -s USE_SDL=2 \
    -s USE_SDL_MIXER=2 \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s INITIAL_MEMORY=67108864 \
    -s STACK_SIZE=5242880 \
    -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
    -s EXPORTED_FUNCTIONS='["_main"]' \
    -s MINIFY_HTML=0 \
    --preload-file ../data@/data \
    --shell-file ../shell.html \
    ../src/main.c \
    ../src/core/engine.c \
    ../src/player/player.c \
    ../src/input/input.c \
    ../src/renderer/raycaster.c \
    ../src/renderer/sprite_renderer.c \
    ../src/renderer/minimap.c \
    ../src/renderer/hud.c \
    ../src/assets/texture.c \
    ../src/assets/sprite.c \
    ../src/assets/image_loader.c \
    ../src/entities/enemy.c \
    ../src/combat/combat.c \
    ../src/combat/weapon.c \
    ../src/audio/sound.c \
    ../src/systems/pickup.c \
    ../src/map/map.c \
    ../src/map/map_loader.c \
    -o raycaster.html

echo "Build complete! Output files in build-wasm/"
echo ""
echo "To run locally, start a web server:"
echo "  cd build-wasm"
echo "  python3 -m http.server 8000"
echo "Then open: http://localhost:8000/raycaster.html"
