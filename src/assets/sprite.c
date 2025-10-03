#include "sprite.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

void sprite_generate_procedural(Texture* texture, int type) {
    memset(texture->data, 0, sizeof(texture->data));

    switch (type) {
        case 0: // Pillar/Column
            for (int y = 0; y < TEXTURE_HEIGHT; y++) {
                for (int x = 0; x < TEXTURE_WIDTH; x++) {
                    // Center circle for pillar
                    int dx = x - TEXTURE_WIDTH / 2;
                    int dy = y - TEXTURE_HEIGHT / 2;
                    int dist = (int)sqrtf(dx * dx + dy * dy);

                    if (dist < 20) {
                        uint32_t gray = 0xA0 - dist * 2;
                        texture->data[y * TEXTURE_WIDTH + x] = (gray << 16) | (gray << 8) | gray | 0xFF000000;
                    } else {
                        texture->data[y * TEXTURE_WIDTH + x] = 0x00000000; // Transparent
                    }
                }
            }
            break;

        case 1: // Green plant/tree
            for (int y = 0; y < TEXTURE_HEIGHT; y++) {
                for (int x = 0; x < TEXTURE_WIDTH; x++) {
                    // Simple tree shape
                    int dx = x - TEXTURE_WIDTH / 2;
                    int dy = y - TEXTURE_HEIGHT / 2;

                    // Trunk
                    if (y > TEXTURE_HEIGHT / 2 && abs(dx) < 4) {
                        texture->data[y * TEXTURE_WIDTH + x] = 0xFF8B4513; // Brown
                    }
                    // Foliage
                    else if (y < TEXTURE_HEIGHT / 2 && dx * dx + dy * dy < 400) {
                        uint32_t green = 0x00 | ((80 + (x + y) % 40) << 8) | 0x00;
                        texture->data[y * TEXTURE_WIDTH + x] = green | 0xFF000000;
                    } else {
                        texture->data[y * TEXTURE_WIDTH + x] = 0x00000000; // Transparent
                    }
                }
            }
            break;

        case 2: // Lamp/Light
            for (int y = 0; y < TEXTURE_HEIGHT; y++) {
                for (int x = 0; x < TEXTURE_WIDTH; x++) {
                    int dx = x - TEXTURE_WIDTH / 2;
                    int dy = y - TEXTURE_HEIGHT / 2;
                    int dist = (int)sqrtf(dx * dx + dy * dy);

                    // Glowing orb
                    if (dist < 16) {
                        uint32_t brightness = 255 - dist * 10;
                        texture->data[y * TEXTURE_WIDTH + x] =
                            (brightness << 16) | (brightness << 8) | 0x00 | 0xFF000000;
                    }
                    // Post
                    else if (y > TEXTURE_HEIGHT / 2 && abs(dx) < 2) {
                        texture->data[y * TEXTURE_WIDTH + x] = 0xFF404040;
                    } else {
                        texture->data[y * TEXTURE_WIDTH + x] = 0x00000000;
                    }
                }
            }
            break;

        case 3: // Barrel
            for (int y = 0; y < TEXTURE_HEIGHT; y++) {
                for (int x = 0; x < TEXTURE_WIDTH; x++) {
                    int dx = abs(x - TEXTURE_WIDTH / 2);
                    int dy = abs(y - TEXTURE_HEIGHT / 2);

                    // Barrel shape (rectangular with bands)
                    if (dx < 20 && dy < 28) {
                        if (y % 16 < 2) {
                            texture->data[y * TEXTURE_WIDTH + x] = 0xFF444444; // Metal band
                        } else {
                            uint32_t brown = 0x66 + (x + y) % 20;
                            texture->data[y * TEXTURE_WIDTH + x] = (brown << 16) | (brown / 2 << 8) | 0x00 | 0xFF000000;
                        }
                    } else {
                        texture->data[y * TEXTURE_WIDTH + x] = 0x00000000;
                    }
                }
            }
            break;

        default: // Red sphere
            for (int y = 0; y < TEXTURE_HEIGHT; y++) {
                for (int x = 0; x < TEXTURE_WIDTH; x++) {
                    int dx = x - TEXTURE_WIDTH / 2;
                    int dy = y - TEXTURE_HEIGHT / 2;
                    int dist = (int)sqrtf(dx * dx + dy * dy);

                    if (dist < 24) {
                        uint32_t red = 200 - dist * 3;
                        texture->data[y * TEXTURE_WIDTH + x] = (red << 16) | 0x00 | 0x00 | 0xFF000000;
                    } else {
                        texture->data[y * TEXTURE_WIDTH + x] = 0x00000000;
                    }
                }
            }
            break;
    }
}

bool sprite_manager_init(SpriteManager* sm) {
    sm->count = 0;
    sm->texture_count = 4;

    // Generate sprite textures
    for (int i = 0; i < sm->texture_count; i++) {
        sprite_generate_procedural(&sm->sprite_textures[i], i);
    }

    // Initialize all sprites as inactive
    for (int i = 0; i < MAX_SPRITES; i++) {
        sm->sprites[i].active = false;
    }

    return true;
}

void sprite_manager_cleanup(SpriteManager* sm) {
    sm->count = 0;
    sm->texture_count = 0;
}

void sprite_add(SpriteManager* sm, float x, float y, int texture_id) {
    if (sm->count >= MAX_SPRITES) {
        return;
    }

    for (int i = 0; i < MAX_SPRITES; i++) {
        if (!sm->sprites[i].active) {
            sm->sprites[i].x = x;
            sm->sprites[i].y = y;
            sm->sprites[i].texture_id = texture_id;
            sm->sprites[i].active = true;
            sm->count++;
            break;
        }
    }
}
