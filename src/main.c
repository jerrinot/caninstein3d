#include "engine.h"
#include "player.h"
#include "raycaster.h"
#include "input.h"
#include "texture.h"
#include "map.h"
#include "sprite.h"
#include "enemy.h"
#include "minimap.h"
#include "combat.h"
#include "hud.h"
#include "sound.h"
#include "pickup.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Global state for Emscripten main loop
typedef struct {
    Engine* engine;
    Player* player;
    TextureManager* texture_manager;
    SpriteManager* sprite_manager;
    EnemyManager* enemy_manager;
    SoundManager* sound_manager;
    PickupManager* pickup_manager;
    Map* map;
    InputState input_state;
    float prev_player_health;  // Track health for damage effects
} GameState;

static GameState g_state;

void main_loop(void) {
    engine_handle_events(g_state.engine);
    engine_update(g_state.engine);

    // Check for game over
    if (!player_is_alive(g_state.player)) {
        g_state.engine->game_over = true;
    }

    // Handle restart
    if (g_state.engine->restart_requested) {
        player_init(g_state.player, g_state.map->player_spawn_x, g_state.map->player_spawn_y);
        g_state.engine->game_over = false;
        g_state.engine->restart_requested = false;
        g_state.prev_player_health = g_state.player->health;
        printf("Game restarted!\n");
    }

    // Don't update game if dead
    if (g_state.engine->game_over) {
        // Still render, but don't process input/updates
        raycaster_render(g_state.engine, g_state.player, g_state.texture_manager, g_state.sprite_manager, g_state.enemy_manager, g_state.pickup_manager);
        minimap_render(g_state.engine, g_state.player, g_state.map, g_state.engine->minimap_enabled);
        hud_render(g_state.engine, g_state.player);
        engine_render(g_state.engine);
        return;
    }

    // Handle input
    input_handle(g_state.player, g_state.engine->delta_time, &g_state.input_state, g_state.sound_manager);
    input_handle_mouse(g_state.player, g_state.engine);

    // Update player
    player_update(g_state.player, g_state.engine->delta_time);

    // Check for damage (trigger visual effects)
    if (g_state.player->health < g_state.prev_player_health) {
        engine_trigger_damage_vignette(g_state.engine);
        engine_trigger_screen_shake(g_state.engine);
    }
    g_state.prev_player_health = g_state.player->health;

    // Handle weapon switching
    if (g_state.input_state.weapon_switch >= 0) {
        player_switch_weapon(g_state.player, g_state.input_state.weapon_switch);
    }

    // Handle shooting
    if (g_state.input_state.shoot_pressed && player_is_alive(g_state.player)) {
        Weapon* weapon = player_get_current_weapon(g_state.player);
        if (weapon_can_fire(weapon)) {
            combat_player_shoot(g_state.player, g_state.enemy_manager, g_state.sound_manager, g_state.pickup_manager);
            engine_trigger_muzzle_flash(g_state.engine);
        }
    }

    // Update enemies
    enemy_manager_update(g_state.enemy_manager, g_state.player, g_state.sound_manager, g_state.pickup_manager, g_state.engine->delta_time);

    // Update pickups
    pickup_manager_update(g_state.pickup_manager, g_state.engine->delta_time);
    pickup_check_collision(g_state.pickup_manager, g_state.player);

    // Render
    raycaster_render(g_state.engine, g_state.player, g_state.texture_manager, g_state.sprite_manager, g_state.enemy_manager, g_state.pickup_manager);
    minimap_render(g_state.engine, g_state.player, g_state.map, g_state.engine->minimap_enabled);
    hud_render(g_state.engine, g_state.player);

    // Apply low health warning effect
    engine_render_low_health_warning(g_state.engine, g_state.player->health, g_state.player->max_health);

    engine_render(g_state.engine);
}

int main(int argc, char* argv[]) {
    Engine engine;
    Player player;
    TextureManager texture_manager;
    SpriteManager sprite_manager;
    EnemyManager enemy_manager;
    SoundManager sound_manager;
    PickupManager pickup_manager;
    Map map;

    // Determine map file
    const char* map_file = "data/maps/test.map";
    if (argc > 1) {
        map_file = argv[1];
    }

    if (!engine_init(&engine)) {
        fprintf(stderr, "Failed to initialize engine\n");
        return 1;
    }

    if (!texture_manager_init(&texture_manager)) {
        fprintf(stderr, "Failed to initialize texture manager\n");
        engine_cleanup(&engine);
        return 1;
    }

    if (!sprite_manager_init(&sprite_manager)) {
        fprintf(stderr, "Failed to initialize sprite manager\n");
        texture_manager_cleanup(&texture_manager);
        engine_cleanup(&engine);
        return 1;
    }

    if (!enemy_manager_init(&enemy_manager)) {
        fprintf(stderr, "Failed to initialize enemy manager\n");
        sprite_manager_cleanup(&sprite_manager);
        texture_manager_cleanup(&texture_manager);
        engine_cleanup(&engine);
        return 1;
    }

    if (!sound_init(&sound_manager)) {
        fprintf(stderr, "Failed to initialize sound system\n");
        enemy_manager_cleanup(&enemy_manager);
        sprite_manager_cleanup(&sprite_manager);
        texture_manager_cleanup(&texture_manager);
        engine_cleanup(&engine);
        return 1;
    }

    if (!pickup_manager_init(&pickup_manager)) {
        fprintf(stderr, "Failed to initialize pickup manager\n");
        sound_cleanup(&sound_manager);
        enemy_manager_cleanup(&enemy_manager);
        sprite_manager_cleanup(&sprite_manager);
        texture_manager_cleanup(&texture_manager);
        engine_cleanup(&engine);
        return 1;
    }

    // Load map first (needed for spawn positions)
    if (!map_load(&map, map_file)) {
        fprintf(stderr, "Failed to load map, using default\n");
        map.player_spawn_x = 22.0f;
        map.player_spawn_y = 12.0f;
    }

    // Try to load enemy sprites (dog)
    if (enemy_load_textures(&enemy_manager, "data/sprites/dog")) {
        printf("Enemy sprites loaded successfully\n");
        // Spawn 6 enemies at random valid positions
        srand(time(NULL));  // Seed random number generator
        int enemies_spawned = 0;
        int max_attempts = 100;

        while (enemies_spawned < 6 && max_attempts > 0) {
            // Random position within map bounds (avoid edges)
            float x = 2.5f + ((float)rand() / RAND_MAX) * 18.0f;
            float y = 2.5f + ((float)rand() / RAND_MAX) * 18.0f;

            // Check if position is valid (not in a wall, not too close to player)
            int grid_x = (int)x;
            int grid_y = (int)y;
            float dx = x - map.player_spawn_x;
            float dy = y - map.player_spawn_y;
            float dist_to_player = sqrtf(dx * dx + dy * dy);

            if (world_map[grid_x][grid_y] == 0 && dist_to_player > 5.0f) {
                // Spawn enemy with variety: 50% normal, 30% fast, 20% tank
                EnemyType type;
                int type_roll = rand() % 100;
                if (type_roll < 50) {
                    type = ENEMY_TYPE_NORMAL;
                } else if (type_roll < 80) {
                    type = ENEMY_TYPE_FAST;
                } else {
                    type = ENEMY_TYPE_TANK;
                }

                enemy_add(&enemy_manager, x, y, type);
                enemies_spawned++;
                const char* type_name = (type == ENEMY_TYPE_FAST) ? "FAST" : (type == ENEMY_TYPE_TANK) ? "TANK" : "NORMAL";
                printf("Spawned %s enemy %d at (%.1f, %.1f)\n", type_name, enemies_spawned, x, y);
            }
            max_attempts--;
        }
        printf("Total enemies spawned: %d\n", enemies_spawned);
    } else {
        fprintf(stderr, "Warning: Enemy sprites not loaded - enemies disabled\n");
    }

    player_init(&player, map.player_spawn_x, map.player_spawn_y);

    // Add some test sprites
    sprite_add(&sprite_manager, 10.5f, 10.5f, 0);  // Pillar
    sprite_add(&sprite_manager, 12.5f, 10.5f, 1);  // Tree
    sprite_add(&sprite_manager, 14.5f, 10.5f, 2);  // Lamp
    sprite_add(&sprite_manager, 16.5f, 10.5f, 3);  // Barrel
    sprite_add(&sprite_manager, 18.5f, 10.5f, 0);  // Pillar
    sprite_add(&sprite_manager, 10.5f, 15.5f, 1);  // Tree
    sprite_add(&sprite_manager, 12.5f, 15.5f, 2);  // Lamp
    sprite_add(&sprite_manager, 6.5f, 6.5f, 3);    // Barrel

    printf("Raycaster Engine Started\n");
    printf("Controls:\n");
    printf("  W/S - Move forward/backward\n");
    printf("  A/D or Arrow Keys - Rotate left/right\n");
    printf("  SPACE or LEFT CLICK - Shoot\n");
    printf("  1-4 - Switch weapons (Knife/Pistol/Shotgun/Machinegun)\n");
    printf("  M - Toggle mouse look\n");
    printf("  TAB - Toggle minimap\n");
    printf("  F11 - Toggle fullscreen\n");
    printf("  ESC - Quit\n");

    // Setup global state for Emscripten
    g_state.engine = &engine;
    g_state.player = &player;
    g_state.texture_manager = &texture_manager;
    g_state.sprite_manager = &sprite_manager;
    g_state.enemy_manager = &enemy_manager;
    g_state.sound_manager = &sound_manager;
    g_state.pickup_manager = &pickup_manager;
    g_state.map = &map;
    g_state.prev_player_health = player.health;

#ifdef __EMSCRIPTEN__
    // Use Emscripten's main loop for browser
    emscripten_set_main_loop(main_loop, 0, 1);
#else
    // Native game loop
    while (engine.running) {
        main_loop();
    }

    map_free(&map);
    pickup_manager_cleanup(&pickup_manager);
    sound_cleanup(&sound_manager);
    enemy_manager_cleanup(&enemy_manager);
    sprite_manager_cleanup(&sprite_manager);
    texture_manager_cleanup(&texture_manager);
    engine_cleanup(&engine);
    printf("Engine shutdown complete\n");
#endif

    return 0;
}
