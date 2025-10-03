#ifndef ENEMY_H
#define ENEMY_H

#include <stdint.h>
#include <stdbool.h>
#include "texture.h"
#include "player.h"
#include "engine.h"
#include "sound.h"
#include "pickup.h"

#define MAX_ENEMIES 32
#define MAX_ENEMY_TEXTURES 8

typedef enum {
    ENEMY_IDLE,
    ENEMY_CHASE,
    ENEMY_ATTACK,
    ENEMY_DEAD
} EnemyState;

typedef enum {
    ENEMY_TYPE_FAST,    // Fast, weak (25 HP, 5 damage, 3.5 speed)
    ENEMY_TYPE_NORMAL,  // Normal (50 HP, 10 damage, 2.5 speed)
    ENEMY_TYPE_TANK,    // Slow, strong (100 HP, 15 damage, 1.5 speed)
    ENEMY_TYPE_COUNT
} EnemyType;

typedef struct {
    float x;                    // Position X
    float y;                    // Position Y
    float spawn_x;              // Original spawn position X
    float spawn_y;              // Original spawn position Y
    float dir_x;                // Direction X (normalized)
    float dir_y;                // Direction Y (normalized)
    float speed;                // Movement speed
    bool active;                // Is enemy in use
    EnemyState state;           // Current AI state
    EnemyType type;             // Enemy type (fast/normal/tank)
    int animation_frame;        // Current animation frame (0-3)
    float animation_time;       // Timer for animation
    float chase_radius;         // Distance to start chasing
    int health;                 // Current health
    int max_health;             // Maximum health
    int damage;                 // Damage dealt to player
    float attack_cooldown;      // Time until can attack again
    float hit_flash_time;       // Time remaining for hit flash effect
    float respawn_timer;        // Time until respawn
} Enemy;

typedef struct {
    Enemy enemies[MAX_ENEMIES];
    int count;
    Texture textures[MAX_ENEMY_TEXTURES];  // Animation frames
    int texture_count;
    float animation_speed;      // Frames per second
    float respawn_time;         // Time in seconds before enemies respawn
    bool respawn_enabled;       // Should enemies respawn
} EnemyManager;

// Enemy manager functions
bool enemy_manager_init(EnemyManager* em);
void enemy_manager_cleanup(EnemyManager* em);
void enemy_manager_update(EnemyManager* em, Player* player, SoundManager* sm, PickupManager* pm, float delta_time);
bool enemy_add(EnemyManager* em, float x, float y, EnemyType type);

// Load enemy textures from directory
bool enemy_load_textures(EnemyManager* em, const char* sprite_dir);

// Combat functions
void enemy_take_damage(Enemy* enemy, int damage, PickupManager* pm);
bool enemy_is_alive(Enemy* enemy);

#endif
