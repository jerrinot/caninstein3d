#include "sound.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Generate a simple procedural sound wave
static Mix_Chunk* generate_sound(int frequency, int duration_ms, float amplitude) {
    int sample_rate = 22050;
    int samples = (sample_rate * duration_ms) / 1000;

    Uint8* buffer = (Uint8*)malloc(samples * sizeof(Uint8));
    if (!buffer) {
        return NULL;
    }

    // Generate simple sine wave with envelope
    for (int i = 0; i < samples; i++) {
        float t = (float)i / sample_rate;
        float envelope = 1.0f - ((float)i / samples); // Fade out
        float value = sinf(2.0f * M_PI * frequency * t) * amplitude * envelope;
        buffer[i] = (Uint8)((value + 1.0f) * 127.5f);
    }

    Mix_Chunk* chunk = Mix_QuickLoad_RAW(buffer, samples);
    if (!chunk) {
        free(buffer);
        return NULL;
    }

    return chunk;
}

// Generate noise-based sound (for gunshots)
static Mix_Chunk* generate_noise_sound(int duration_ms, float amplitude) {
    int sample_rate = 22050;
    int samples = (sample_rate * duration_ms) / 1000;

    Uint8* buffer = (Uint8*)malloc(samples * sizeof(Uint8));
    if (!buffer) {
        return NULL;
    }

    // Generate white noise with envelope
    for (int i = 0; i < samples; i++) {
        float envelope = 1.0f - ((float)i / samples); // Quick fade out
        envelope = envelope * envelope; // Sharper envelope for gunshot
        float noise = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        float value = noise * amplitude * envelope;
        buffer[i] = (Uint8)((value + 1.0f) * 127.5f);
    }

    Mix_Chunk* chunk = Mix_QuickLoad_RAW(buffer, samples);
    if (!chunk) {
        free(buffer);
        return NULL;
    }

    return chunk;
}

bool sound_init(SoundManager* sm) {
    // Initialize SDL_mixer
    if (Mix_OpenAudio(22050, AUDIO_U8, 1, 2048) < 0) {
        fprintf(stderr, "SDL_mixer initialization failed: %s\n", Mix_GetError());
        return false;
    }

    // Allocate mixing channels
    Mix_AllocateChannels(16);

    // Generate procedural sounds
    sm->sounds[SOUND_PISTOL_SHOOT] = generate_noise_sound(100, 0.3f);
    sm->sounds[SOUND_SHOTGUN_SHOOT] = generate_noise_sound(150, 0.5f);
    sm->sounds[SOUND_MACHINEGUN_SHOOT] = generate_noise_sound(80, 0.25f);
    sm->sounds[SOUND_ENEMY_HIT] = generate_sound(400, 100, 0.3f);
    sm->sounds[SOUND_ENEMY_DEATH] = generate_sound(200, 300, 0.4f);
    sm->sounds[SOUND_PLAYER_DAMAGE] = generate_sound(150, 200, 0.5f);
    sm->sounds[SOUND_FOOTSTEP] = generate_noise_sound(30, 0.15f);  // Short, quiet
    sm->sounds[SOUND_LOW_AMMO] = generate_sound(800, 150, 0.2f);   // High beep

    // Check if all sounds were generated
    for (int i = 0; i < SOUND_COUNT; i++) {
        if (!sm->sounds[i]) {
            fprintf(stderr, "Failed to generate sound %d\n", i);
            sound_cleanup(sm);
            return false;
        }
    }

    sm->initialized = true;
    printf("Sound system initialized with procedural audio\n");
    return true;
}

void sound_cleanup(SoundManager* sm) {
    if (!sm->initialized) {
        return;
    }

    for (int i = 0; i < SOUND_COUNT; i++) {
        if (sm->sounds[i]) {
            Mix_FreeChunk(sm->sounds[i]);
            sm->sounds[i] = NULL;
        }
    }

    Mix_CloseAudio();
    sm->initialized = false;
}

void sound_play(SoundManager* sm, SoundType type) {
    if (!sm->initialized || type < 0 || type >= SOUND_COUNT) {
        return;
    }

    if (sm->sounds[type]) {
        Mix_PlayChannel(-1, sm->sounds[type], 0);
    }
}
