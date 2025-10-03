#ifndef PICKUP_H
#define PICKUP_H

#include <stdbool.h>
#include "player.h"
#include "texture.h"

#define MAX_PICKUPS 32

typedef enum {
    PICKUP_AMMO_SMALL,   // +20 ammo
    PICKUP_AMMO_LARGE,   // +50 ammo
    PICKUP_HEALTH_SMALL, // +25 health
    PICKUP_HEALTH_LARGE, // +50 health
    PICKUP_COUNT
} PickupType;

typedef struct {
    float x;
    float y;
    PickupType type;
    bool active;
    float lifetime;      // Time before disappearing (0 = permanent)
} Pickup;

typedef struct {
    Pickup pickups[MAX_PICKUPS];
    int count;
    Texture textures[4];  // Textures for each pickup type
} PickupManager;

// Initialize pickup manager
bool pickup_manager_init(PickupManager* pm);

// Cleanup pickup manager
void pickup_manager_cleanup(PickupManager* pm);

// Update pickups (handle lifetime)
void pickup_manager_update(PickupManager* pm, float delta_time);

// Add a pickup to the world
bool pickup_add(PickupManager* pm, float x, float y, PickupType type, float lifetime);

// Check for pickup collision with player
void pickup_check_collision(PickupManager* pm, Player* player);

#endif // PICKUP_H
