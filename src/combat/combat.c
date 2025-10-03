#include "combat.h"
#include "map.h"
#include "sound.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define M_PI 3.14159265358979323846
#define DEG_TO_RAD(deg) ((deg) * M_PI / 180.0f)

ShotResult combat_fire_shot(Player* player, EnemyManager* em, float angle_offset) {
    ShotResult result;
    result.hit = false;
    result.enemy_index = -1;
    result.distance = 0.0f;

    // Calculate ray direction with angle offset
    float angle = angle_offset * DEG_TO_RAD(1);
    float ray_dir_x = player->dir_x * cosf(angle) - player->dir_y * sinf(angle);
    float ray_dir_y = player->dir_x * sinf(angle) + player->dir_y * cosf(angle);

    // DDA algorithm (same as raycaster)
    int map_x = (int)player->x;
    int map_y = (int)player->y;

    float delta_dist_x = (ray_dir_x == 0) ? 1e30 : fabsf(1 / ray_dir_x);
    float delta_dist_y = (ray_dir_y == 0) ? 1e30 : fabsf(1 / ray_dir_y);

    int step_x = (ray_dir_x < 0) ? -1 : 1;
    int step_y = (ray_dir_y < 0) ? -1 : 1;

    float side_dist_x = (ray_dir_x < 0) ?
        (player->x - map_x) * delta_dist_x :
        (map_x + 1.0 - player->x) * delta_dist_x;

    float side_dist_y = (ray_dir_y < 0) ?
        (player->y - map_y) * delta_dist_y :
        (map_y + 1.0 - player->y) * delta_dist_y;

    float closest_enemy_dist = 1e30;
    int closest_enemy_index = -1;

    // Cast ray until we hit a wall (max 100 steps)
    for (int step = 0; step < 100; step++) {
        // Check for enemy hits at current position
        for (int i = 0; i < MAX_ENEMIES; i++) {
            Enemy* enemy = &em->enemies[i];
            if (!enemy->active || enemy->state == ENEMY_DEAD) {
                continue;
            }

            // Calculate distance from ray to enemy
            float to_enemy_x = enemy->x - player->x;
            float to_enemy_y = enemy->y - player->y;
            float enemy_dist = sqrtf(to_enemy_x * to_enemy_x + to_enemy_y * to_enemy_y);

            // Project enemy position onto ray
            float dot = to_enemy_x * ray_dir_x + to_enemy_y * ray_dir_y;

            // Skip if enemy is behind player
            if (dot < 0) {
                continue;
            }

            // Calculate perpendicular distance from ray to enemy
            float closest_x = player->x + ray_dir_x * dot;
            float closest_y = player->y + ray_dir_y * dot;
            float perp_dist = sqrtf(
                (enemy->x - closest_x) * (enemy->x - closest_x) +
                (enemy->y - closest_y) * (enemy->y - closest_y)
            );

            // Hit if within enemy radius (0.3 units)
            if (perp_dist < 0.3f && enemy_dist < closest_enemy_dist) {
                closest_enemy_dist = enemy_dist;
                closest_enemy_index = i;
            }
        }

        // Step to next grid square
        if (side_dist_x < side_dist_y) {
            side_dist_x += delta_dist_x;
            map_x += step_x;
        } else {
            side_dist_y += delta_dist_y;
            map_y += step_y;
        }

        // Check if we hit a wall
        if (map_x < 0 || map_x >= MAP_WIDTH || map_y < 0 || map_y >= MAP_HEIGHT) {
            break;  // Out of bounds
        }

        if (world_map[map_x][map_y] > 0) {
            break;  // Hit wall
        }
    }

    // Return closest enemy hit
    if (closest_enemy_index >= 0) {
        result.hit = true;
        result.enemy_index = closest_enemy_index;
        result.distance = closest_enemy_dist;
    }

    return result;
}

void combat_player_shoot(Player* player, EnemyManager* em, SoundManager* sm, PickupManager* pm) {
    Weapon* weapon = player_get_current_weapon(player);

    // Check if can fire
    if (!weapon_can_fire(weapon)) {
        if (weapon->ammo == 0 && weapon->max_ammo != -1) {
            printf("*Click* Out of ammo!\n");
        }
        return;
    }

    // Fire the weapon
    weapon_fire(weapon);
    printf("BANG! Fired %s (Ammo: %d/%d)\n",
           weapon->name,
           weapon->ammo == -1 ? -1 : weapon->ammo,
           weapon->max_ammo);

    // Play weapon sound
    if (sm) {
        switch (weapon->type) {
            case WEAPON_PISTOL:
                sound_play(sm, SOUND_PISTOL_SHOOT);
                break;
            case WEAPON_SHOTGUN:
                sound_play(sm, SOUND_SHOTGUN_SHOOT);
                break;
            case WEAPON_MACHINEGUN:
                sound_play(sm, SOUND_MACHINEGUN_SHOOT);
                break;
            default:
                break;
        }

        // Low ammo warning (10% or less ammo remaining)
        if (weapon->max_ammo > 0 && weapon->ammo > 0 && weapon->ammo <= weapon->max_ammo / 10) {
            sound_play(sm, SOUND_LOW_AMMO);
        }
    }

    // Handle different weapon types
    if (weapon->has_spread && weapon->pellet_count > 1) {
        // Shotgun: fire multiple pellets
        int hits = 0;
        bool got_kill = false;
        for (int i = 0; i < weapon->pellet_count; i++) {
            // Random spread within angle
            float spread = ((float)rand() / RAND_MAX - 0.5f) * weapon->spread_angle * 2.0f;
            ShotResult result = combat_fire_shot(player, em, spread);

            if (result.hit) {
                // Check range (shotgun has limited range)
                if (weapon->range > 0.0f && result.distance > weapon->range) {
                    continue;  // Too far
                }

                Enemy* enemy = &em->enemies[result.enemy_index];
                int old_health = enemy->health;
                enemy_take_damage(enemy, weapon->damage, pm);

                // Play sound for first hit only
                if (hits == 0 && sm) {
                    if (old_health > 0 && enemy->health <= 0) {
                        sound_play(sm, SOUND_ENEMY_DEATH);
                    } else {
                        sound_play(sm, SOUND_ENEMY_HIT);
                    }
                }

                // Track kills on first pellet that kills
                if (old_health > 0 && enemy->health <= 0 && !got_kill) {
                    player->kills++;
                    player->score += 10;
                    got_kill = true;
                }
                hits++;
            }
        }
        if (hits > 0) {
            printf("Hit enemy with %d pellets!\n", hits);
            if (got_kill) {
                printf("KILL! Total: %d | Score: %d\n", player->kills, player->score);
            }
        }
    } else {
        // Single shot (pistol, machinegun, knife)
        float spread = 0.0f;
        if (weapon->has_spread) {
            // Random inaccuracy
            spread = ((float)rand() / RAND_MAX - 0.5f) * weapon->spread_angle * 2.0f;
        }

        ShotResult result = combat_fire_shot(player, em, spread);

        if (result.hit) {
            // Check range (knife has limited range)
            if (weapon->range > 0.0f && result.distance > weapon->range) {
                printf("Too far away!\n");
                return;
            }

            Enemy* enemy = &em->enemies[result.enemy_index];
            int old_health = enemy->health;
            enemy_take_damage(enemy, weapon->damage, pm);

            // Play hit/death sound
            if (sm) {
                if (old_health > 0 && enemy->health <= 0) {
                    sound_play(sm, SOUND_ENEMY_DEATH);
                } else {
                    sound_play(sm, SOUND_ENEMY_HIT);
                }
            }

            // Track kills and score
            if (old_health > 0 && enemy->health <= 0) {
                player->kills++;
                player->score += 10;  // 10 points per kill
                printf("KILL! Total: %d | Score: %d\n", player->kills, player->score);
            } else {
                printf("Hit enemy! Distance: %.1f\n", result.distance);
            }
        } else {
            printf("Missed!\n");
        }
    }
}
