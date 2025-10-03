# Raycaster Engine

A Wolfenstein 3D-style FPS engine built from scratch in C using SDL2.

## Building

### Prerequisites

- GCC or Clang compiler
- SDL2 development libraries
- CMake 3.10+

On Ubuntu/Debian:
```bash
sudo apt-get install build-essential libsdl2-dev cmake
```

On Fedora:
```bash
sudo dnf install gcc SDL2-devel cmake
```

On Arch:
```bash
sudo pacman -S base-devel sdl2 cmake
```

### Compilation

```bash
cmake -S . -B cmake-build-debug
cmake --build cmake-build-debug
```

Or use your IDE (CLion, VS Code, etc.) to open the CMakeLists.txt

### Running

**IMPORTANT:** Run from the project root directory so the engine can find `data/maps/test.map`

Using the run script (easiest):
```bash
./run.sh
```

Or manually:
```bash
./cmake-build-debug/raycaster
```

Load a custom map:
```bash
./run.sh data/maps/your_map.map
```

### Cleaning

```bash
rm -rf cmake-build-debug
# or from your IDE: Build → Clean
```

## Controls

- **W** - Move forward
- **S** - Move backward
- **A / Left Arrow** - Rotate left
- **D / Right Arrow** - Rotate right
- **M** - Toggle mouse look
- **TAB** - Toggle minimap
- **ESC** - Quit

## Architecture

```
/src
  /core       - Game loop, timing, SDL initialization
  /renderer   - Raycasting algorithm, wall and sprite rendering
  /input      - Keyboard input handling
  /map        - Level data, collision detection, map loader
  /player     - Player movement and camera
  /assets     - Texture and sprite generation
/include      - Header files
/data
  /maps       - Level files (.map format)
  /textures   - Texture assets (procedurally generated)
```

## How It Works

The engine uses **raycasting** to create a 3D perspective:

### Wall Rendering
1. For each vertical column on screen, cast a ray from the player
2. Use DDA (Digital Differential Analysis) to find the first wall hit
3. Calculate perpendicular distance to avoid fish-eye distortion
4. Extract vertical texture slice and scale to wall height
5. Apply side-based brightness for depth perception
6. Store distance in Z-buffer for sprite rendering

### Sprite Rendering
1. Calculate distance from player to each sprite
2. Sort sprites from far to near (painter's algorithm)
3. Transform sprite positions to camera space
4. Project sprites to screen coordinates
5. Draw sprites with transparency, checking Z-buffer for occlusion

This is the same technique used in Wolfenstein 3D (1992).