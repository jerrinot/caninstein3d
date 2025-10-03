#ifndef SOUND_H
#define SOUND_H

#include <stdbool.h>
#include <SDL2/SDL_mixer.h>

typedef enum {
    SOUND_PISTOL_SHOOT,
    SOUND_SHOTGUN_SHOOT,
    SOUND_MACHINEGUN_SHOOT,
    SOUND_ENEMY_HIT,
    SOUND_ENEMY_DEATH,
    SOUND_PLAYER_DAMAGE,
    SOUND_FOOTSTEP,
    SOUND_LOW_AMMO,
    SOUND_COUNT
} SoundType;

typedef struct {
    Mix_Chunk* sounds[SOUND_COUNT];
    bool initialized;
} SoundManager;

// Initialize sound system
bool sound_init(SoundManager* sm);

// Cleanup sound system
void sound_cleanup(SoundManager* sm);

// Play a sound effect
void sound_play(SoundManager* sm, SoundType type);

#endif // SOUND_H
