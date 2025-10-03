#include "enemy.h"
#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

bool enemy_manager_init(EnemyManager* em) {
    memset(em->enemies, 0, sizeof(em->enemies));
    em->count = 0;
    em->texture_count = 0;
    em->animation_speed = 10.0f; // 10 FPS animation
    em->respawn_time = 2.0f;     // 2 seconds to respawn (reduced from 5s)
    em->respawn_enabled = true;  // Respawning enabled by default

    // Mark all enemies as inactive
    for (int i = 0; i < MAX_ENEMIES; i++) {
        em->enemies[i].active = false;
    }

    return true;
}

void enemy_manager_cleanup(EnemyManager* em) {
    em->count = 0;
    em->texture_count = 0;
}

bool enemy_load_textures(EnemyManager* em, const char* sprite_dir) {
    // Try to load walk animation frames (walk_0.png, walk_1.png, etc.)
    const char* frame_names[] = {"walk_0.png", "walk_1.png", "walk_2.png", "walk_3.png"};

    for (int i = 0; i < 4 && i < MAX_ENEMY_TEXTURES; i++) {
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "%s/%s", sprite_dir, frame_names[i]);

        if (texture_load_from_file(&em->textures[i], filepath)) {
            em->texture_count++;
            printf("Loaded enemy texture: %s\n", filepath);
        } else {
            fprintf(stderr, "Warning: Could not load %s\n", filepath);
        }
    }

    if (em->texture_count == 0) {
        fprintf(stderr, "Error: No enemy textures loaded from %s\n", sprite_dir);
        return false;
    }

    return true;
}

bool enemy_add(EnemyManager* em, float x, float y, EnemyType type) {
    if (em->count >= MAX_ENEMIES) {
        fprintf(stderr, "Cannot add enemy: MAX_ENEMIES reached\n");
        return false;
    }

    // Find first inactive slot
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!em->enemies[i].active) {
            Enemy* e = &em->enemies[i];
            e->x = x;
            e->y = y;
            e->spawn_x = x;  // Remember spawn position
            e->spawn_y = y;
            e->dir_x = 0.0f;
            e->dir_y = 0.0f;
            e->active = true;
            e->state = ENEMY_IDLE;
            e->type = type;
            e->animation_frame = 0;
            e->animation_time = 0.0f;
            e->chase_radius = 10.0f;  // Start chasing within 10 tiles
            e->attack_cooldown = 0.0f;
            e->hit_flash_time = 0.0f;
            e->respawn_timer = 0.0f;

            // Set stats based on type
            switch (type) {
                case ENEMY_TYPE_FAST:
                    e->speed = 3.5f;
                    e->health = 25;
                    e->max_health = 25;
                    e->damage = 5;
                    break;
                case ENEMY_TYPE_NORMAL:
                    e->speed = 2.5f;
                    e->health = 50;
                    e->max_health = 50;
                    e->damage = 10;
                    break;
                case ENEMY_TYPE_TANK:
                    e->speed = 1.5f;
                    e->health = 100;
                    e->max_health = 100;
                    e->damage = 15;
                    break;
                default:
                    e->speed = 2.5f;
                    e->health = 50;
                    e->max_health = 50;
                    e->damage = 10;
                    break;
            }

            em->count++;
            return true;
        }
    }

    return false;
}

void enemy_manager_update(EnemyManager* em, Player* player, SoundManager* sm, PickupManager* pm, float delta_time) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* e = &em->enemies[i];
        if (!e->active) continue;

        // Update attack cooldown
        if (e->attack_cooldown > 0.0f) {
            e->attack_cooldown -= delta_time;
            if (e->attack_cooldown < 0.0f) {
                e->attack_cooldown = 0.0f;
            }
        }

        // Update hit flash timer
        if (e->hit_flash_time > 0.0f) {
            e->hit_flash_time -= delta_time;
            if (e->hit_flash_time < 0.0f) {
                e->hit_flash_time = 0.0f;
            }
        }

        // Handle respawning
        if (e->state == ENEMY_DEAD) {
            if (em->respawn_enabled) {
                e->respawn_timer += delta_time;
                if (e->respawn_timer >= em->respawn_time) {
                    // Check if spawn point is far enough from player
                    float dx = e->spawn_x - player->x;
                    float dy = e->spawn_y - player->y;
                    float dist = sqrtf(dx * dx + dy * dy);

                    // Only respawn if player is at least 8 tiles away from spawn point
                    if (dist >= 8.0f) {
                        e->x = e->spawn_x;
                        e->y = e->spawn_y;
                        e->health = e->max_health;
                        e->state = ENEMY_IDLE;
                        e->respawn_timer = 0.0f;
                        e->dir_x = 0.0f;
                        e->dir_y = 0.0f;
                        e->animation_frame = 0;
                        e->animation_time = 0.0f;
                        printf("Enemy respawned at (%.1f, %.1f)!\n", e->spawn_x, e->spawn_y);
                    }
                    // If player is too close, wait and check again next frame
                }
            }
            continue;  // Skip AI if dead
        }

        // Calculate distance to player
        float dx = player->x - e->x;
        float dy = player->y - e->y;
        float distance = sqrtf(dx * dx + dy * dy);

        // State machine
        if (distance < 0.5f) {
            // Close enough to attack
            e->state = ENEMY_ATTACK;
        } else if (distance < e->chase_radius) {
            e->state = ENEMY_CHASE;
        } else {
            e->state = ENEMY_IDLE;
        }

        // AI behavior
        switch (e->state) {
            case ENEMY_IDLE:
                // Stand still
                e->dir_x = 0.0f;
                e->dir_y = 0.0f;
                break;

            case ENEMY_CHASE:
                // Move toward player
                if (distance > 0.1f) {
                    // Normalize direction
                    e->dir_x = dx / distance;
                    e->dir_y = dy / distance;

                    // Calculate new position
                    float new_x = e->x + e->dir_x * e->speed * delta_time;
                    float new_y = e->y + e->dir_y * e->speed * delta_time;

                    // Simple collision detection (same as player)
                    if (world_map[(int)new_x][(int)e->y] == 0) {
                        e->x = new_x;
                    }
                    if (world_map[(int)e->x][(int)new_y] == 0) {
                        e->y = new_y;
                    }

                    // Update animation
                    e->animation_time += delta_time;
                    if (e->animation_time >= 1.0f / em->animation_speed) {
                        e->animation_time = 0.0f;
                        e->animation_frame = (e->animation_frame + 1) % em->texture_count;
                    }
                }
                break;

            case ENEMY_ATTACK:
                // Attack player if cooldown is ready
                if (e->attack_cooldown <= 0.0f) {
                    player_take_damage(player, e->damage);
                    if (sm) {
                        sound_play(sm, SOUND_PLAYER_DAMAGE);
                    }
                    e->attack_cooldown = 1.0f;  // 1 second between attacks
                }
                break;

            case ENEMY_DEAD:
                // Do nothing
                break;
        }
    }
}

void enemy_take_damage(Enemy* enemy, int damage, PickupManager* pm) {
    if (enemy->state == ENEMY_DEAD) {
        return;  // Already dead
    }

    enemy->health -= damage;
    enemy->hit_flash_time = 0.15f;  // Flash white for 150ms
    printf("Enemy took %d damage! Health: %d/%d\n", damage, enemy->health, enemy->max_health);

    if (enemy->health <= 0) {
        enemy->health = 0;
        enemy->state = ENEMY_DEAD;
        enemy->respawn_timer = 0.0f;  // Start respawn timer
        printf("Enemy killed! Will respawn in %.1f seconds\n", 2.0f);

        // Drop pickups on death
        if (pm) {
            int roll = rand() % 100;
            if (roll < 40) {
                // 40% chance: Drop ammo
                PickupType ammo_type = (rand() % 2 == 0) ? PICKUP_AMMO_SMALL : PICKUP_AMMO_LARGE;
                pickup_add(pm, enemy->x, enemy->y, ammo_type, 10.0f);  // Disappears after 10 seconds
                printf("Dropped ammo at (%.1f, %.1f)\n", enemy->x, enemy->y);
            } else if (roll < 55) {
                // 15% chance: Drop health
                PickupType health_type = (rand() % 2 == 0) ? PICKUP_HEALTH_SMALL : PICKUP_HEALTH_LARGE;
                pickup_add(pm, enemy->x, enemy->y, health_type, 10.0f);
                printf("Dropped health at (%.1f, %.1f)\n", enemy->x, enemy->y);
            }
        }
    }
}

bool enemy_is_alive(Enemy* enemy) {
    return enemy->active && enemy->state != ENEMY_DEAD;
}
