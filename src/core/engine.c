#include "engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

// Forward declaration
static bool engine_resize(Engine* engine, int width, int height);

#ifdef __EMSCRIPTEN__
// Fullscreen change callback for Emscripten
static EM_BOOL fullscreen_callback(int eventType, const EmscriptenFullscreenChangeEvent *e, void *userData) {
    Engine* engine = (Engine*)userData;

    printf("Fullscreen callback: isFullscreen=%d, element=%dx%d, screen=%dx%d\n",
           e->isFullscreen, e->elementWidth, e->elementHeight, e->screenWidth, e->screenHeight);

    if (e->isFullscreen) {
        // Entering fullscreen - use screen dimensions
        int width = e->screenWidth;
        int height = e->screenHeight;

        // Update SDL window size
        SDL_SetWindowSize(engine->window, width, height);

        engine_resize(engine, width, height);
        printf("Fullscreen resize applied: %dx%d\n", width, height);
    } else {
        // Exiting fullscreen - resize back to default
        SDL_SetWindowSize(engine->window, DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT);
        engine_resize(engine, DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT);
        printf("Windowed resize: %dx%d\n", DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT);
    }

    return EM_TRUE;
}
#endif

static bool engine_resize(Engine* engine, int width, int height) {
    // Free old pixel buffer and texture
    free(engine->pixels);
    SDL_DestroyTexture(engine->texture);

    // Update dimensions
    engine->screen_width = width;
    engine->screen_height = height;

    // Create new texture
    engine->texture = SDL_CreateTexture(
        engine->renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        width,
        height
    );

    if (!engine->texture) {
        fprintf(stderr, "Texture recreation failed: %s\n", SDL_GetError());
        return false;
    }

    // Allocate new pixel buffer
    engine->pixels = (uint32_t*)malloc(width * height * sizeof(uint32_t));
    if (!engine->pixels) {
        fprintf(stderr, "Memory reallocation failed for pixel buffer\n");
        return false;
    }

    return true;
}

bool engine_init(Engine* engine) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return false;
    }

    // Initialize with default dimensions
    engine->screen_width = DEFAULT_SCREEN_WIDTH;
    engine->screen_height = DEFAULT_SCREEN_HEIGHT;

    engine->window = SDL_CreateWindow(
        "Caninestein3D",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        engine->screen_width,
        engine->screen_height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED
    );

    if (!engine->window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    engine->renderer = SDL_CreateRenderer(
        engine->window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!engine->renderer) {
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(engine->window);
        SDL_Quit();
        return false;
    }

    engine->texture = SDL_CreateTexture(
        engine->renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        engine->screen_width,
        engine->screen_height
    );

    if (!engine->texture) {
        fprintf(stderr, "Texture creation failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(engine->renderer);
        SDL_DestroyWindow(engine->window);
        SDL_Quit();
        return false;
    }

    engine->pixels = (uint32_t*)malloc(engine->screen_width * engine->screen_height * sizeof(uint32_t));
    if (!engine->pixels) {
        fprintf(stderr, "Memory allocation failed for pixel buffer\n");
        SDL_DestroyTexture(engine->texture);
        SDL_DestroyRenderer(engine->renderer);
        SDL_DestroyWindow(engine->window);
        SDL_Quit();
        return false;
    }

    engine->running = true;
    engine->last_time = SDL_GetTicks();
    engine->delta_time = 0.0f;
    engine->mouse_captured = false;
    engine->mouse_sensitivity = 100;
    engine->minimap_enabled = true;
    engine->fullscreen = false;

    // Visual effects
    engine->muzzle_flash_time = 0.0f;
    engine->damage_vignette_time = 0.0f;
    engine->screen_shake_time = 0.0f;
    engine->shake_offset_x = 0;
    engine->shake_offset_y = 0;

    // Game state
    engine->game_over = false;
    engine->restart_requested = false;

#ifdef __EMSCRIPTEN__
    // Register fullscreen change callback for browser
    emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, engine, 1, fullscreen_callback);
#endif

    return true;
}

void engine_cleanup(Engine* engine) {
    free(engine->pixels);
    SDL_DestroyTexture(engine->texture);
    SDL_DestroyRenderer(engine->renderer);
    SDL_DestroyWindow(engine->window);
    SDL_Quit();
}

void engine_handle_events(Engine* engine) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            engine->running = false;
        }
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                engine->running = false;
            }
            // Toggle mouse capture with M key
            if (event.key.keysym.sym == SDLK_m) {
                engine->mouse_captured = !engine->mouse_captured;
                SDL_SetRelativeMouseMode(engine->mouse_captured ? SDL_TRUE : SDL_FALSE);
                if (engine->mouse_captured) {
                    SDL_ShowCursor(SDL_DISABLE);
                    printf("Mouse look ENABLED\n");
                } else {
                    SDL_ShowCursor(SDL_ENABLE);
                    printf("Mouse look DISABLED\n");
                }
            }
            // Toggle minimap with TAB key
            if (event.key.keysym.sym == SDLK_TAB) {
                engine->minimap_enabled = !engine->minimap_enabled;
            }
            // Restart game with R key (when dead)
            if (event.key.keysym.sym == SDLK_r && engine->game_over) {
                engine->restart_requested = true;
                printf("Restart requested\n");
            }
            // Toggle fullscreen with F11 key
            if (event.key.keysym.sym == SDLK_F11) {
                engine->fullscreen = !engine->fullscreen;
#ifdef __EMSCRIPTEN__
                // Browser fullscreen - request on canvas element
                if (engine->fullscreen) {
                    EmscriptenFullscreenStrategy strategy;
                    strategy.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT;
                    strategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF;
                    strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
                    emscripten_request_fullscreen_strategy("#canvas", 1, &strategy);
                    printf("Fullscreen ENABLED (browser)\n");
                } else {
                    emscripten_exit_fullscreen();
                    printf("Fullscreen DISABLED (browser)\n");
                }
#else
                // Native fullscreen
                if (engine->fullscreen) {
                    SDL_SetWindowFullscreen(engine->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                    SDL_GetWindowSize(engine->window, &engine->screen_width, &engine->screen_height);
                    if (!engine_resize(engine, engine->screen_width, engine->screen_height)) {
                        fprintf(stderr, "Failed to resize for fullscreen\n");
                    }
                    printf("Fullscreen ENABLED (%dx%d)\n", engine->screen_width, engine->screen_height);
                } else {
                    SDL_SetWindowFullscreen(engine->window, 0);
                    printf("Fullscreen DISABLED\n");
                }
#endif
            }
        }
        // Handle window resize
        if (event.type == SDL_WINDOWEVENT) {
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                int new_width = event.window.data1;
                int new_height = event.window.data2;
                if (!engine_resize(engine, new_width, new_height)) {
                    fprintf(stderr, "Failed to resize engine\n");
                    engine->running = false;
                }
            }
        }
    }
}

void engine_update(Engine* engine) {
    uint32_t current_time = SDL_GetTicks();
    engine->delta_time = (current_time - engine->last_time) / 1000.0f;
    engine->last_time = current_time;

    // Update visual effects
    if (engine->muzzle_flash_time > 0.0f) {
        engine->muzzle_flash_time -= engine->delta_time;
        if (engine->muzzle_flash_time < 0.0f) {
            engine->muzzle_flash_time = 0.0f;
        }
    }

    if (engine->damage_vignette_time > 0.0f) {
        engine->damage_vignette_time -= engine->delta_time;
        if (engine->damage_vignette_time < 0.0f) {
            engine->damage_vignette_time = 0.0f;
        }
    }

    if (engine->screen_shake_time > 0.0f) {
        engine->screen_shake_time -= engine->delta_time;
        if (engine->screen_shake_time < 0.0f) {
            engine->screen_shake_time = 0.0f;
            engine->shake_offset_x = 0;
            engine->shake_offset_y = 0;
        } else {
            // Random shake
            engine->shake_offset_x = (rand() % 5) - 2;
            engine->shake_offset_y = (rand() % 5) - 2;
        }
    }
}

void engine_render(Engine* engine) {
    // Apply muzzle flash (brighten entire screen)
    if (engine->muzzle_flash_time > 0.0f) {
        float intensity = engine->muzzle_flash_time / 0.05f;  // 0.05s duration
        for (int i = 0; i < engine->screen_width * engine->screen_height; i++) {
            uint32_t pixel = engine->pixels[i];
            uint32_t r = ((pixel >> 16) & 0xFF);
            uint32_t g = ((pixel >> 8) & 0xFF);
            uint32_t b = (pixel & 0xFF);

            r = r + (255 - r) * intensity * 0.5f;
            g = g + (255 - g) * intensity * 0.5f;
            b = b + (255 - b) * intensity * 0.5f;

            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;

            engine->pixels[i] = (r << 16) | (g << 8) | b;
        }
    }

    // Apply damage vignette (red edges)
    if (engine->damage_vignette_time > 0.0f) {
        float intensity = engine->damage_vignette_time / 0.5f;  // 0.5s duration
        for (int y = 0; y < engine->screen_height; y++) {
            for (int x = 0; x < engine->screen_width; x++) {
                // Calculate distance from edge
                float dx = (float)x / engine->screen_width - 0.5f;
                float dy = (float)y / engine->screen_height - 0.5f;
                float dist = sqrtf(dx * dx + dy * dy);
                float edge_factor = (dist - 0.3f) / 0.2f;
                if (edge_factor < 0.0f) edge_factor = 0.0f;
                if (edge_factor > 1.0f) edge_factor = 1.0f;

                if (edge_factor > 0.0f) {
                    int idx = y * engine->screen_width + x;
                    uint32_t pixel = engine->pixels[idx];
                    uint32_t r = ((pixel >> 16) & 0xFF);
                    uint32_t g = ((pixel >> 8) & 0xFF);
                    uint32_t b = (pixel & 0xFF);

                    r = r + (255 - r) * edge_factor * intensity;
                    g = g * (1.0f - edge_factor * intensity);
                    b = b * (1.0f - edge_factor * intensity);

                    if (r > 255) r = 255;
                    engine->pixels[idx] = (r << 16) | (g << 8) | b;
                }
            }
        }
    }

    SDL_UpdateTexture(engine->texture, NULL, engine->pixels, engine->screen_width * sizeof(uint32_t));
    SDL_RenderClear(engine->renderer);
    SDL_RenderCopy(engine->renderer, engine->texture, NULL, NULL);
    SDL_RenderPresent(engine->renderer);
}

void engine_trigger_muzzle_flash(Engine* engine) {
    engine->muzzle_flash_time = 0.05f;  // 50ms flash
}

void engine_trigger_damage_vignette(Engine* engine) {
    engine->damage_vignette_time = 0.5f;  // 500ms fade
}

void engine_trigger_screen_shake(Engine* engine) {
    engine->screen_shake_time = 0.2f;  // 200ms shake (doubled for better feedback)
}

void engine_render_low_health_warning(Engine* engine, int player_health, int max_health) {
    // Show red pulse when health is below 25%
    if (player_health <= 0 || player_health > max_health / 4) {
        return;
    }

    // Pulse effect based on time
    float pulse = sinf((float)SDL_GetTicks() / 300.0f) * 0.5f + 0.5f;  // 0.0 to 1.0
    float intensity = pulse * 0.4f;  // Max 40% intensity

    // Apply red tint to edges
    for (int y = 0; y < engine->screen_height; y++) {
        for (int x = 0; x < engine->screen_width; x++) {
            float dx = (float)x / engine->screen_width - 0.5f;
            float dy = (float)y / engine->screen_height - 0.5f;
            float dist = sqrtf(dx * dx + dy * dy);
            float edge_factor = (dist - 0.2f) / 0.3f;
            if (edge_factor < 0.0f) edge_factor = 0.0f;
            if (edge_factor > 1.0f) edge_factor = 1.0f;

            if (edge_factor > 0.0f) {
                int idx = y * engine->screen_width + x;
                uint32_t pixel = engine->pixels[idx];
                uint32_t r = ((pixel >> 16) & 0xFF);
                uint32_t g = ((pixel >> 8) & 0xFF);
                uint32_t b = (pixel & 0xFF);

                float effect = edge_factor * intensity;
                r = r + (255 - r) * effect;
                g = g * (1.0f - effect);
                b = b * (1.0f - effect);

                if (r > 255) r = 255;
                engine->pixels[idx] = (r << 16) | (g << 8) | b;
            }
        }
    }
}
