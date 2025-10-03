#ifndef COMBAT_H
#define COMBAT_H

#include "player.h"
#include "enemy.h"
#include "weapon.h"
#include "sound.h"
#include <stdbool.h>

// Result of a raycast shot
typedef struct {
    bool hit;           // Did we hit an enemy?
    int enemy_index;    // Index of hit enemy
    float distance;     // Distance to hit
} ShotResult;

// Fire a raycast shot and check for enemy hits
ShotResult combat_fire_shot(Player* player, EnemyManager* em, float angle_offset);

// Fire weapon and handle all combat logic
void combat_player_shoot(Player* player, EnemyManager* em, SoundManager* sm, PickupManager* pm);

#endif
