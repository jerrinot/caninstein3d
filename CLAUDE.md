# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a Wolfenstein 3D-style raycaster FPS engine built from scratch in C using SDL2. It's an educational project demonstrating classic raycasting techniques with software rendering.

## Build & Run Commands

### Build
```bash
cmake -S . -B cmake-build-debug
cmake --build cmake-build-debug
```

### Run
**IMPORTANT:** Always run from project root so the engine can find `data/maps/test.map`
```bash
./run.sh                              # Recommended
./cmake-build-debug/raycaster         # Direct execution
./run.sh data/maps/custom.map         # Load custom map
```

### Clean
```bash
rm -rf cmake-build-debug
```

## Architecture Overview

### Rendering Pipeline
The engine uses a **two-pass rendering system**:

1. **Wall Rendering Pass** (`src/renderer/raycaster.c`)
   - Casts 640 rays (one per screen column)
   - DDA algorithm traces rays through 2D grid until wall hit
   - Calculates perpendicular wall distance (fish-eye correction)
   - Samples wall textures vertically and scales to screen height
   - Stores distances in **Z-buffer** for sprite occlusion

2. **Sprite Rendering Pass** (`src/renderer/sprite_renderer.c`)
   - Sorts all active sprites by distance (painter's algorithm)
   - Transforms sprite positions to camera space using inverse camera matrix
   - Projects to screen coordinates as billboards
   - Checks Z-buffer per pixel to handle occlusion behind walls
   - Skips transparent pixels (alpha channel)

### Core Systems

**Engine Loop** (`src/core/engine.c`)
- SDL window management and pixel buffer (640x480 ARGB)
- Game loop with delta timing for frame-rate independence
- Event handling and rendering pipeline coordination

**Player System** (`src/player/player.c`)
- Position stored as float (x, y) on 2D grid
- Direction vector (dir_x, dir_y) for looking direction
- Camera plane vector (plane_x, plane_y) defines FOV (~66°)
- Collision uses grid-based checks against `world_map[x][y]`

**Map System** (`src/map/map.c`, `src/map/map_loader.c`)
- Global `world_map[24][24]` array stores tile IDs
- 0 = empty, 1-5 = wall types (mapped to textures)
- Text-based `.map` format: dimensions, grid data, spawn position
- Parser supports comments and custom spawn points

**Asset System** (`src/assets/`)
- All textures/sprites are **procedurally generated** (no external files)
- Wall textures: brick, stone, tile patterns (64x64)
- Sprite textures: pillar, tree, lamp, barrel (64x64 with transparency)

### Key Data Structures

**Player** (`include/player.h`)
```c
typedef struct {
    float x, y;              // World position
    float dir_x, dir_y;      // Direction vector
    float plane_x, plane_y;  // Camera plane (FOV)
    float move_speed, rot_speed;
} Player;
```

**Sprite** (`include/sprite.h`)
```c
typedef struct {
    float x, y;              // World position
    int texture_id;          // Which sprite texture to use
    bool active;             // Is sprite visible
} Sprite;
```

**Engine** (`include/engine.h`)
```c
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    uint32_t* pixels;        // 640x480 framebuffer
    bool running;
    uint32_t last_time;
    float delta_time;
} Engine;
```

## Critical Implementation Details

### Raycasting Math
- **DDA Algorithm**: Steps through grid tiles efficiently to find wall intersections
- **Fish-eye Correction**: Uses perpendicular distance (not Euclidean) to prevent warping
- **Texture Mapping**: Calculates exact wall hit position (0.0-1.0) to sample texture X coordinate

### Sprite Billboarding
The inverse camera matrix transformation is critical for correct sprite positioning:
```c
float inv_det = 1.0f / (plane_x * dir_y - dir_x * plane_y);
float transform_x = inv_det * (dir_y * sprite_x - dir_x * sprite_y);
float transform_y = inv_det * (-plane_y * sprite_x + plane_x * sprite_y);
```
`transform_y` is the depth (distance), `transform_x` determines horizontal screen position.

### Map Format
```
24 24              # Width Height
1 1 1 1 ...        # Grid data (space-separated integers)
...
22.0 12.0          # Player spawn X Y (optional)
```

### Coordinate Systems
- **World coordinates**: Float positions on 2D grid (e.g., 12.5, 10.3)
- **Grid coordinates**: Integer tile indices (e.g., [12][10])
- **Screen coordinates**: Pixel positions (0-639, 0-479)

## Code Conventions

- **C99 standard** enforced by CMakeLists.txt
- Headers in `include/`, implementations in `src/`
- Global `world_map` array accessed by raycaster and player collision
- All assets generated at runtime (no file I/O except maps)
- Delta time used for movement: `position += direction * speed * delta_time`

## Adding New Features

### Adding Wall Textures
Edit `src/assets/texture.c::texture_generate_procedural()` to add new procedural texture patterns. Increase `MAX_TEXTURES` in `include/texture.h` if needed.

### Adding Sprite Types
Edit `src/assets/sprite.c::sprite_generate_procedural()` to add new sprite appearances. Add sprites to scene in `src/main.c` using `sprite_add(&sprite_manager, x, y, texture_id)`.

### Creating New Maps
Create `.map` file in `data/maps/` following format above. Run with `./run.sh data/maps/yourmap.map`.

## Known Limitations

- Map size fixed at 24x24 (see `MAP_WIDTH`/`MAP_HEIGHT` in `include/map.h`)
- Max 128 sprites (`MAX_SPRITES` in `include/sprite.h`)
- Max 8 wall textures (`MAX_TEXTURES` in `include/texture.h`)
- No floor/ceiling texture mapping (solid colors only)
- No doors or animated walls yet
- Sprites don't animate (single frame only)

## Project Status

Phases 1-6 complete (foundation, raycasting, textures, player, maps, sprites). Phase 7 (polish) planned but not implemented: minimap, mouse look, performance optimizations.
