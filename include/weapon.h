#ifndef WEAPON_H
#define WEAPON_H

#include <stdbool.h>

typedef enum {
    WEAPON_KNIFE,
    WEAPON_PISTOL,
    WEAPON_SHOTGUN,
    WEAPON_MACHINEGUN,
    WEAPON_COUNT
} WeaponType;

typedef struct {
    WeaponType type;
    const char* name;
    int damage;
    int ammo;           // Current ammo in reserve
    int max_ammo;       // Max ammo capacity
    float fire_rate;    // Seconds between shots
    float cooldown;     // Time until can fire again
    float range;        // Max effective range (0 = infinite)
    bool has_spread;    // Shotgun spread pattern
    float spread_angle; // Degrees of spread
    int pellet_count;   // Number of pellets for shotgun
} Weapon;

// Initialize a weapon of given type
void weapon_init(Weapon* weapon, WeaponType type);

// Check if weapon can fire
bool weapon_can_fire(Weapon* weapon);

// Fire the weapon (consume ammo, set cooldown)
void weapon_fire(Weapon* weapon);

// Update weapon cooldown
void weapon_update(Weapon* weapon, float delta_time);

// Add ammo to weapon (returns amount actually added)
int weapon_add_ammo(Weapon* weapon, int amount);

// Get weapon name string
const char* weapon_get_name(WeaponType type);

#endif
