#include "player.h"
#include "map.h"
#include <stdio.h>

void player_init(Player* player, float x, float y) {
    player->x = x;
    player->y = y;
    player->dir_x = -1.0f;
    player->dir_y = 0.0f;
    player->plane_x = 0.0f;
    player->plane_y = 0.66f;  // FOV ~66 degrees
    player->move_speed = 5.0f;
    player->rot_speed = 2.0f;  // Reduced for smoother rotation

    // Combat stats
    player->health = 100;
    player->max_health = 100;
    player->damage_cooldown = 0.0f;

    // Initialize weapons
    for (int i = 0; i < WEAPON_COUNT; i++) {
        weapon_init(&player->weapons[i], (WeaponType)i);
    }

    // Start with pistol (more fun than knife)
    player->current_weapon_index = WEAPON_PISTOL;

    // Stats
    player->kills = 0;
    player->score = 0;

    // Footstep
    player->footstep_timer = 0.0f;
}

void player_move_forward(Player* player, float delta_time, SoundManager* sm) {
    float new_x = player->x + player->dir_x * player->move_speed * delta_time;
    float new_y = player->y + player->dir_y * player->move_speed * delta_time;

    bool moved = false;
    if (world_map[(int)new_x][(int)player->y] == 0) {
        player->x = new_x;
        moved = true;
    }
    if (world_map[(int)player->x][(int)new_y] == 0) {
        player->y = new_y;
        moved = true;
    }

    // Play footstep sound every 0.4 seconds while moving
    if (moved && sm) {
        player->footstep_timer -= delta_time;
        if (player->footstep_timer <= 0.0f) {
            sound_play(sm, SOUND_FOOTSTEP);
            player->footstep_timer = 0.4f;
        }
    }
}

void player_move_backward(Player* player, float delta_time, SoundManager* sm) {
    float new_x = player->x - player->dir_x * player->move_speed * delta_time;
    float new_y = player->y - player->dir_y * player->move_speed * delta_time;

    bool moved = false;
    if (world_map[(int)new_x][(int)player->y] == 0) {
        player->x = new_x;
        moved = true;
    }
    if (world_map[(int)player->x][(int)new_y] == 0) {
        player->y = new_y;
        moved = true;
    }

    // Play footstep sound every 0.4 seconds while moving
    if (moved && sm) {
        player->footstep_timer -= delta_time;
        if (player->footstep_timer <= 0.0f) {
            sound_play(sm, SOUND_FOOTSTEP);
            player->footstep_timer = 0.4f;
        }
    }
}

void player_rotate_left(Player* player, float delta_time) {
    float rot = player->rot_speed * delta_time;
    float old_dir_x = player->dir_x;
    player->dir_x = player->dir_x * cosf(rot) - player->dir_y * sinf(rot);
    player->dir_y = old_dir_x * sinf(rot) + player->dir_y * cosf(rot);

    float old_plane_x = player->plane_x;
    player->plane_x = player->plane_x * cosf(rot) - player->plane_y * sinf(rot);
    player->plane_y = old_plane_x * sinf(rot) + player->plane_y * cosf(rot);
}

void player_rotate_right(Player* player, float delta_time) {
    float rot = -player->rot_speed * delta_time;
    float old_dir_x = player->dir_x;
    player->dir_x = player->dir_x * cosf(rot) - player->dir_y * sinf(rot);
    player->dir_y = old_dir_x * sinf(rot) + player->dir_y * cosf(rot);

    float old_plane_x = player->plane_x;
    player->plane_x = player->plane_x * cosf(rot) - player->plane_y * sinf(rot);
    player->plane_y = old_plane_x * sinf(rot) + player->plane_y * cosf(rot);
}

void player_take_damage(Player* player, int damage) {
    // Don't take damage if in cooldown period
    if (player->damage_cooldown > 0.0f) {
        return;
    }

    player->health -= damage;
    if (player->health < 0) {
        player->health = 0;
    }

    // Set damage cooldown (0.5 seconds of invulnerability)
    player->damage_cooldown = 0.5f;

    printf("Player took %d damage! Health: %d/%d\n", damage, player->health, player->max_health);

    // Note: Visual effects will be triggered in main loop
}

void player_switch_weapon(Player* player, int weapon_index) {
    if (weapon_index < 0 || weapon_index >= WEAPON_COUNT) {
        return;
    }

    player->current_weapon_index = weapon_index;
    printf("Switched to %s\n", weapon_get_name((WeaponType)weapon_index));
}

Weapon* player_get_current_weapon(Player* player) {
    return &player->weapons[player->current_weapon_index];
}

void player_update(Player* player, float delta_time) {
    // Update damage cooldown
    if (player->damage_cooldown > 0.0f) {
        player->damage_cooldown -= delta_time;
        if (player->damage_cooldown < 0.0f) {
            player->damage_cooldown = 0.0f;
        }
    }

    // Update all weapons
    for (int i = 0; i < WEAPON_COUNT; i++) {
        weapon_update(&player->weapons[i], delta_time);
    }
}

bool player_is_alive(Player* player) {
    return player->health > 0;
}
