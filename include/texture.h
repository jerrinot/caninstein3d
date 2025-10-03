#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>
#include <stdbool.h>

#define TEXTURE_WIDTH 64
#define TEXTURE_HEIGHT 64
#define MAX_TEXTURES 8

typedef struct {
    uint32_t data[TEXTURE_WIDTH * TEXTURE_HEIGHT];
} Texture;

typedef struct {
    Texture textures[MAX_TEXTURES];
    int count;
} TextureManager;

bool texture_manager_init(TextureManager* tm);
void texture_manager_cleanup(TextureManager* tm);
void texture_generate_procedural(Texture* texture, int type);
bool texture_load_from_file(Texture* texture, const char* filepath);
uint32_t texture_get_pixel(Texture* texture, int x, int y);

#endif
