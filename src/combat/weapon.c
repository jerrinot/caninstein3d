#include "weapon.h"
#include <string.h>

void weapon_init(Weapon* weapon, WeaponType type) {
    weapon->type = type;
    weapon->cooldown = 0.0f;

    switch (type) {
        case WEAPON_KNIFE:
            weapon->name = "Knife";
            weapon->damage = 15;
            weapon->ammo = -1;  // Infinite
            weapon->max_ammo = -1;
            weapon->fire_rate = 0.5f;
            weapon->range = 1.5f;
            weapon->has_spread = false;
            weapon->spread_angle = 0.0f;
            weapon->pellet_count = 1;
            break;

        case WEAPON_PISTOL:
            weapon->name = "Pistol";
            weapon->damage = 25;
            weapon->ammo = 100;  // Increased from 40
            weapon->max_ammo = 200;
            weapon->fire_rate = 0.3f;
            weapon->range = 0.0f;  // Infinite
            weapon->has_spread = false;
            weapon->spread_angle = 0.0f;
            weapon->pellet_count = 1;
            break;

        case WEAPON_SHOTGUN:
            weapon->name = "Shotgun";
            weapon->damage = 10;  // Per pellet (7 pellets = 70 total at close range)
            weapon->ammo = 20;  // Increased from 12
            weapon->max_ammo = 50;
            weapon->fire_rate = 1.0f;
            weapon->range = 8.0f;
            weapon->has_spread = true;
            weapon->spread_angle = 5.0f;  // Degrees
            weapon->pellet_count = 7;
            break;

        case WEAPON_MACHINEGUN:
            weapon->name = "Machinegun";
            weapon->damage = 12;  // Reduced from 15 for balance
            weapon->ammo = 150;
            weapon->max_ammo = 300;
            weapon->fire_rate = 0.15f;  // Increased from 0.1f (slower = 6.67 shots/sec instead of 10)
            weapon->range = 0.0f;  // Infinite
            weapon->has_spread = true;
            weapon->spread_angle = 4.0f;  // Increased spread for less accuracy
            weapon->pellet_count = 1;
            break;

        default:
            weapon_init(weapon, WEAPON_KNIFE);
            break;
    }
}

bool weapon_can_fire(Weapon* weapon) {
    // Check cooldown
    if (weapon->cooldown > 0.0f) {
        return false;
    }

    // Check ammo (knife has infinite ammo)
    if (weapon->ammo == -1) {
        return true;  // Infinite ammo
    }

    return weapon->ammo > 0;
}

void weapon_fire(Weapon* weapon) {
    // Consume ammo
    if (weapon->ammo > 0) {
        weapon->ammo--;
    }

    // Set cooldown
    weapon->cooldown = weapon->fire_rate;
}

void weapon_update(Weapon* weapon, float delta_time) {
    if (weapon->cooldown > 0.0f) {
        weapon->cooldown -= delta_time;
        if (weapon->cooldown < 0.0f) {
            weapon->cooldown = 0.0f;
        }
    }
}

int weapon_add_ammo(Weapon* weapon, int amount) {
    if (weapon->max_ammo == -1) {
        return 0;  // Infinite ammo, can't add more
    }

    int old_ammo = weapon->ammo;
    weapon->ammo += amount;

    if (weapon->ammo > weapon->max_ammo) {
        weapon->ammo = weapon->max_ammo;
    }

    return weapon->ammo - old_ammo;  // Return actual amount added
}

const char* weapon_get_name(WeaponType type) {
    switch (type) {
        case WEAPON_KNIFE: return "Knife";
        case WEAPON_PISTOL: return "Pistol";
        case WEAPON_SHOTGUN: return "Shotgun";
        case WEAPON_MACHINEGUN: return "Machinegun";
        default: return "Unknown";
    }
}
