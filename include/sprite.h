#ifndef SPRITE_H
#define SPRITE_H

#include <stdint.h>
#include <stdbool.h>
#include "texture.h"

#define MAX_SPRITES 128

typedef struct {
    float x;          // Position X
    float y;          // Position Y
    int texture_id;   // Texture index
    bool active;      // Is sprite active
} Sprite;

typedef struct {
    Sprite sprites[MAX_SPRITES];
    int count;
    Texture sprite_textures[8];  // Sprite textures
    int texture_count;
} SpriteManager;

bool sprite_manager_init(SpriteManager* sm);
void sprite_manager_cleanup(SpriteManager* sm);
void sprite_add(SpriteManager* sm, float x, float y, int texture_id);
void sprite_generate_procedural(Texture* texture, int type);

#endif
