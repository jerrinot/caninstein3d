#include "pickup.h"
#include "weapon.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// Generate procedural pickup textures
static void generate_pickup_texture(Texture* tex, PickupType type) {
    for (int y = 0; y < TEXTURE_HEIGHT; y++) {
        for (int x = 0; x < TEXTURE_WIDTH; x++) {
            uint32_t color = 0x00000000;  // Transparent by default

            switch (type) {
                case PICKUP_AMMO_SMALL: {
                    // Small ammo box - yellow/gold box with bullets
                    int cx = TEXTURE_WIDTH / 2;
                    int cy = TEXTURE_HEIGHT / 2;
                    int dx = abs(x - cx);
                    int dy = abs(y - cy);

                    // Box outline (16x16)
                    if (dx <= 8 && dy <= 8) {
                        if (dx == 8 || dy == 8) {
                            color = 0xFF886600;  // Dark orange border
                        } else {
                            color = 0xFFFFCC00;  // Yellow/gold fill
                            // Add bullets pattern
                            if (dx < 3 && (dy % 4 == 0)) {
                                color = 0xFF996600;  // Bullet lines
                            }
                        }
                    }
                    break;
                }

                case PICKUP_AMMO_LARGE: {
                    // Large ammo box - bigger yellow box with more detail
                    int cx = TEXTURE_WIDTH / 2;
                    int cy = TEXTURE_HEIGHT / 2;
                    int dx = abs(x - cx);
                    int dy = abs(y - cy);

                    // Box outline (24x24)
                    if (dx <= 12 && dy <= 12) {
                        if (dx == 12 || dy == 12) {
                            color = 0xFF886600;  // Dark orange border
                        } else {
                            color = 0xFFFFCC00;  // Yellow/gold fill
                            // Add bullets pattern
                            if ((dx < 4 && dy % 3 == 0) || (dx >= 8 && dy % 3 == 0)) {
                                color = 0xFF996600;  // Bullet lines
                            }
                        }
                    }
                    break;
                }

                case PICKUP_HEALTH_SMALL: {
                    // Small health - red cross
                    int cx = TEXTURE_WIDTH / 2;
                    int cy = TEXTURE_HEIGHT / 2;
                    int dx = abs(x - cx);
                    int dy = abs(y - cy);

                    // Cross pattern (8x8 with 2 pixel thick arms)
                    if ((dx <= 1 && dy <= 4) || (dx <= 4 && dy <= 1)) {
                        color = 0xFFFF0000;  // Bright red
                    } else if ((dx == 2 && dy <= 4) || (dx <= 4 && dy == 2)) {
                        color = 0xFFCC0000;  // Dark red outline
                    }
                    // White background circle
                    else if (dx * dx + dy * dy <= 64) {
                        color = 0xFFFFFFFF;  // White
                    }
                    break;
                }

                case PICKUP_HEALTH_LARGE: {
                    // Large health - bigger red cross with border
                    int cx = TEXTURE_WIDTH / 2;
                    int cy = TEXTURE_HEIGHT / 2;
                    int dx = abs(x - cx);
                    int dy = abs(y - cy);

                    // Cross pattern (12x12 with 3 pixel thick arms)
                    if ((dx <= 2 && dy <= 6) || (dx <= 6 && dy <= 2)) {
                        color = 0xFFFF0000;  // Bright red
                    } else if ((dx == 3 && dy <= 6) || (dx <= 6 && dy == 3)) {
                        color = 0xFFCC0000;  // Dark red outline
                    }
                    // White background circle
                    else if (dx * dx + dy * dy <= 100) {
                        color = 0xFFFFFFFF;  // White
                    }
                    break;
                }

                default:
                    color = 0xFFFFFFFF;  // White fallback
                    break;
            }

            tex->data[y * TEXTURE_WIDTH + x] = color;
        }
    }
}

bool pickup_manager_init(PickupManager* pm) {
    memset(pm->pickups, 0, sizeof(pm->pickups));
    pm->count = 0;

    // Mark all pickups as inactive
    for (int i = 0; i < MAX_PICKUPS; i++) {
        pm->pickups[i].active = false;
    }

    // Generate textures for each pickup type
    generate_pickup_texture(&pm->textures[PICKUP_AMMO_SMALL], PICKUP_AMMO_SMALL);
    generate_pickup_texture(&pm->textures[PICKUP_AMMO_LARGE], PICKUP_AMMO_LARGE);
    generate_pickup_texture(&pm->textures[PICKUP_HEALTH_SMALL], PICKUP_HEALTH_SMALL);
    generate_pickup_texture(&pm->textures[PICKUP_HEALTH_LARGE], PICKUP_HEALTH_LARGE);

    return true;
}

void pickup_manager_cleanup(PickupManager* pm) {
    pm->count = 0;
}

void pickup_manager_update(PickupManager* pm, float delta_time) {
    for (int i = 0; i < MAX_PICKUPS; i++) {
        Pickup* p = &pm->pickups[i];
        if (!p->active) continue;

        // Update lifetime
        if (p->lifetime > 0.0f) {
            p->lifetime -= delta_time;
            if (p->lifetime <= 0.0f) {
                p->active = false;
                pm->count--;
            }
        }
    }
}

bool pickup_add(PickupManager* pm, float x, float y, PickupType type, float lifetime) {
    if (pm->count >= MAX_PICKUPS) {
        return false;
    }

    // Find first inactive slot
    for (int i = 0; i < MAX_PICKUPS; i++) {
        if (!pm->pickups[i].active) {
            Pickup* p = &pm->pickups[i];
            p->x = x;
            p->y = y;
            p->type = type;
            p->active = true;
            p->lifetime = lifetime;
            pm->count++;
            return true;
        }
    }

    return false;
}

void pickup_check_collision(PickupManager* pm, Player* player) {
    for (int i = 0; i < MAX_PICKUPS; i++) {
        Pickup* p = &pm->pickups[i];
        if (!p->active) continue;

        // Check distance to player
        float dx = p->x - player->x;
        float dy = p->y - player->y;
        float dist = sqrtf(dx * dx + dy * dy);

        // Collision radius
        if (dist < 0.5f) {
            switch (p->type) {
                case PICKUP_AMMO_SMALL: {
                    Weapon* weapon = player_get_current_weapon(player);
                    int added = weapon_add_ammo(weapon, 20);
                    if (added > 0) {
                        printf("Picked up +%d ammo\n", added);
                        p->active = false;
                        pm->count--;
                    }
                    break;
                }

                case PICKUP_AMMO_LARGE: {
                    Weapon* weapon = player_get_current_weapon(player);
                    int added = weapon_add_ammo(weapon, 50);
                    if (added > 0) {
                        printf("Picked up +%d ammo\n", added);
                        p->active = false;
                        pm->count--;
                    }
                    break;
                }

                case PICKUP_HEALTH_SMALL:
                    if (player->health < player->max_health) {
                        player->health += 25;
                        if (player->health > player->max_health) {
                            player->health = player->max_health;
                        }
                        printf("Picked up +25 health (Health: %d)\n", player->health);
                        p->active = false;
                        pm->count--;
                    }
                    break;

                case PICKUP_HEALTH_LARGE:
                    if (player->health < player->max_health) {
                        player->health += 50;
                        if (player->health > player->max_health) {
                            player->health = player->max_health;
                        }
                        printf("Picked up +50 health (Health: %d)\n", player->health);
                        p->active = false;
                        pm->count--;
                    }
                    break;

                default:
                    break;
            }
        }
    }
}
