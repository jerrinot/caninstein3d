#ifndef ENGINE_H
#define ENGINE_H

#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdbool.h>

// Default screen dimensions
#define DEFAULT_SCREEN_WIDTH 640
#define DEFAULT_SCREEN_HEIGHT 480

// Engine state structure
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    uint32_t* pixels;
    int screen_width;
    int screen_height;
    bool running;
    uint32_t last_time;
    float delta_time;
    bool mouse_captured;
    int mouse_sensitivity;
    bool minimap_enabled;
    bool fullscreen;

    // Visual effects
    float muzzle_flash_time;    // Time remaining for muzzle flash
    float damage_vignette_time; // Time remaining for damage vignette
    float screen_shake_time;    // Time remaining for screen shake
    int shake_offset_x;         // Screen shake offset X
    int shake_offset_y;         // Screen shake offset Y

    // Game state
    bool game_over;             // Is game over
    bool restart_requested;     // Player pressed R to restart
} Engine;

// Engine functions
bool engine_init(Engine* engine);
void engine_cleanup(Engine* engine);
void engine_handle_events(Engine* engine);
void engine_update(Engine* engine);
void engine_render(Engine* engine);
void engine_render_low_health_warning(Engine* engine, int player_health, int max_health);

// Visual effect triggers
void engine_trigger_muzzle_flash(Engine* engine);
void engine_trigger_damage_vignette(Engine* engine);
void engine_trigger_screen_shake(Engine* engine);

#endif
