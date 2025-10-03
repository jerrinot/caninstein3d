#ifndef PLAYER_H
#define PLAYER_H

#include <math.h>
#include "weapon.h"
#include "sound.h"

// Player structure
typedef struct {
    float x;        // X position
    float y;        // Y position
    float dir_x;    // Direction vector X
    float dir_y;    // Direction vector Y
    float plane_x;  // Camera plane X
    float plane_y;  // Camera plane Y
    float move_speed;
    float rot_speed;

    // Combat stats
    int health;
    int max_health;
    float damage_cooldown;  // Invulnerability time after taking damage

    // Weapons
    Weapon weapons[WEAPON_COUNT];
    int current_weapon_index;

    // Stats
    int kills;
    int score;

    // Footstep sound
    float footstep_timer;
} Player;

void player_init(Player* player, float x, float y);
void player_move_forward(Player* player, float delta_time, SoundManager* sm);
void player_move_backward(Player* player, float delta_time, SoundManager* sm);
void player_rotate_left(Player* player, float delta_time);
void player_rotate_right(Player* player, float delta_time);

// Combat functions
void player_take_damage(Player* player, int damage);
void player_switch_weapon(Player* player, int weapon_index);
Weapon* player_get_current_weapon(Player* player);
void player_update(Player* player, float delta_time);
bool player_is_alive(Player* player);

#endif
