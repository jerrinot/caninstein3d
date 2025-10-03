#include "texture.h"
#include "image_loader.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

uint32_t texture_get_pixel(Texture* texture, int x, int y) {
    if (x < 0 || x >= TEXTURE_WIDTH || y < 0 || y >= TEXTURE_HEIGHT) {
        return 0xFF00FF; // Magenta for out of bounds
    }
    return texture->data[y * TEXTURE_WIDTH + x];
}

// Generate procedural textures
void texture_generate_procedural(Texture* texture, int type) {
    memset(texture->data, 0, sizeof(texture->data));

    switch (type) {
        case 1: // Red brick wall
            for (int y = 0; y < TEXTURE_HEIGHT; y++) {
                for (int x = 0; x < TEXTURE_WIDTH; x++) {
                    // Create brick pattern
                    int brick_y = y / 16;
                    int brick_x = x / 16;
                    int offset = (brick_y % 2) * 8;

                    // Mortar lines
                    if (y % 16 == 0 || ((x + offset) % 16 == 0)) {
                        texture->data[y * TEXTURE_WIDTH + x] = 0x444444; // Dark gray mortar
                    } else {
                        // Red brick with some variation
                        uint32_t red = 0xAA + ((x + y) % 30);
                        uint32_t variation = ((x * y) % 20);
                        texture->data[y * TEXTURE_WIDTH + x] = (red << 16) | (variation << 8) | 0x00;
                    }
                }
            }
            break;

        case 2: // Green stone
            for (int y = 0; y < TEXTURE_HEIGHT; y++) {
                for (int x = 0; x < TEXTURE_WIDTH; x++) {
                    uint32_t green = 0x50 + ((x + y * 2) % 40);
                    uint32_t noise = ((x * 7 + y * 11) % 30);
                    texture->data[y * TEXTURE_WIDTH + x] = (noise << 16) | (green << 8) | noise;
                }
            }
            break;

        case 3: // Blue tile
            for (int y = 0; y < TEXTURE_HEIGHT; y++) {
                for (int x = 0; x < TEXTURE_WIDTH; x++) {
                    // Tile pattern
                    if ((x % 32 < 2) || (y % 32 < 2)) {
                        texture->data[y * TEXTURE_WIDTH + x] = 0x222244; // Dark blue grout
                    } else {
                        uint32_t blue = 0x80 + ((x + y) % 40);
                        texture->data[y * TEXTURE_WIDTH + x] = 0x002200 | blue;
                    }
                }
            }
            break;

        case 4: // White/gray stone
            for (int y = 0; y < TEXTURE_HEIGHT; y++) {
                for (int x = 0; x < TEXTURE_WIDTH; x++) {
                    uint32_t gray = 0xC0 + ((x * y) % 40);
                    texture->data[y * TEXTURE_WIDTH + x] = (gray << 16) | (gray << 8) | gray;
                }
            }
            break;

        case 5: // Yellow/gold
            for (int y = 0; y < TEXTURE_HEIGHT; y++) {
                for (int x = 0; x < TEXTURE_WIDTH; x++) {
                    uint32_t val = 0xB0 + ((x + y * 3) % 30);
                    texture->data[y * TEXTURE_WIDTH + x] = (val << 16) | (val << 8) | 0x00;
                }
            }
            break;

        default: // Checkerboard pattern
            for (int y = 0; y < TEXTURE_HEIGHT; y++) {
                for (int x = 0; x < TEXTURE_WIDTH; x++) {
                    if (((x / 8) + (y / 8)) % 2 == 0) {
                        texture->data[y * TEXTURE_WIDTH + x] = 0xFFFFFF;
                    } else {
                        texture->data[y * TEXTURE_WIDTH + x] = 0x000000;
                    }
                }
            }
            break;
    }
}

bool texture_manager_init(TextureManager* tm) {
    tm->count = MAX_TEXTURES;

    // Generate procedural textures for each wall type
    for (int i = 0; i < MAX_TEXTURES; i++) {
        texture_generate_procedural(&tm->textures[i], i);
    }

    return true;
}

void texture_manager_cleanup(TextureManager* tm) {
    tm->count = 0;
}

bool texture_load_from_file(Texture* texture, const char* filepath) {
    int width, height;
    uint32_t* pixels = image_load(filepath, &width, &height);

    if (!pixels) {
        fprintf(stderr, "Failed to load texture from: %s\n", filepath);
        return false;
    }

    // Check if dimensions match
    if (width != TEXTURE_WIDTH || height != TEXTURE_HEIGHT) {
        fprintf(stderr, "Texture size mismatch: %s (expected %dx%d, got %dx%d)\n",
                filepath, TEXTURE_WIDTH, TEXTURE_HEIGHT, width, height);
        image_free(pixels);
        return false;
    }

    // Copy pixel data
    memcpy(texture->data, pixels, TEXTURE_WIDTH * TEXTURE_HEIGHT * sizeof(uint32_t));
    image_free(pixels);

    return true;
}
