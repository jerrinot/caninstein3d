#include "raycaster.h"
#include "sprite.h"
#include "enemy.h"
#include "engine.h"
#include "player.h"
#include <stdlib.h>
#include <math.h>

typedef struct {
    float distance;
    int sprite_index;
    int type;  // 0=static sprite, 1=enemy, 2=pickup
} SpriteOrder;

static int compare_sprites(const void* a, const void* b) {
    SpriteOrder* sa = (SpriteOrder*)a;
    SpriteOrder* sb = (SpriteOrder*)b;
    if (sa->distance > sb->distance) return -1;
    if (sa->distance < sb->distance) return 1;
    return 0;
}

void render_sprites(Engine* engine, Player* player, SpriteManager* sm, EnemyManager* em, PickupManager* pm, float* z_buffer) {
    // Calculate sprite distances and sort
    // Combined static sprites + enemies + pickups
    SpriteOrder sprite_order[MAX_SPRITES + MAX_ENEMIES + MAX_PICKUPS];
    int sprite_count = 0;

    // Add static sprites
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (!sm->sprites[i].active) continue;

        sprite_order[sprite_count].sprite_index = i;
        sprite_order[sprite_count].type = 0;  // Static sprite
        sprite_order[sprite_count].distance =
            (player->x - sm->sprites[i].x) * (player->x - sm->sprites[i].x) +
            (player->y - sm->sprites[i].y) * (player->y - sm->sprites[i].y);
        sprite_count++;
    }

    // Add enemy sprites
    if (em) {
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!em->enemies[i].active) continue;

            sprite_order[sprite_count].sprite_index = i;
            sprite_order[sprite_count].type = 1;  // Enemy
            sprite_order[sprite_count].distance =
                (player->x - em->enemies[i].x) * (player->x - em->enemies[i].x) +
                (player->y - em->enemies[i].y) * (player->y - em->enemies[i].y);
            sprite_count++;
        }
    }

    // Add pickup sprites
    if (pm) {
        for (int i = 0; i < MAX_PICKUPS; i++) {
            if (!pm->pickups[i].active) continue;

            sprite_order[sprite_count].sprite_index = i;
            sprite_order[sprite_count].type = 2;  // Pickup
            sprite_order[sprite_count].distance =
                (player->x - pm->pickups[i].x) * (player->x - pm->pickups[i].x) +
                (player->y - pm->pickups[i].y) * (player->y - pm->pickups[i].y);
            sprite_count++;
        }
    }

    // Sort all sprites from far to near
    qsort(sprite_order, sprite_count, sizeof(SpriteOrder), compare_sprites);

    // Render each sprite
    for (int i = 0; i < sprite_count; i++) {
        float sprite_x, sprite_y;
        int tex_id;

        // Get sprite position and texture based on type
        if (sprite_order[i].type == 1) {  // Enemy
            Enemy* enemy = &em->enemies[sprite_order[i].sprite_index];
            sprite_x = enemy->x;
            sprite_y = enemy->y;
            tex_id = enemy->animation_frame % em->texture_count;
        } else if (sprite_order[i].type == 2) {  // Pickup
            Pickup* pickup = &pm->pickups[sprite_order[i].sprite_index];
            sprite_x = pickup->x;
            sprite_y = pickup->y;
            tex_id = pickup->type;  // Use pickup type as texture index
        } else {  // Static sprite
            Sprite* sprite = &sm->sprites[sprite_order[i].sprite_index];
            sprite_x = sprite->x;
            sprite_y = sprite->y;
            tex_id = sprite->texture_id;
        }

        // Translate sprite position to relative to camera
        float rel_x = sprite_x - player->x;
        float rel_y = sprite_y - player->y;

        // Transform sprite with inverse camera matrix
        float inv_det = 1.0f / (player->plane_x * player->dir_y - player->dir_x * player->plane_y);
        float transform_x = inv_det * (player->dir_y * rel_x - player->dir_x * rel_y);
        float transform_y = inv_det * (-player->plane_y * rel_x + player->plane_x * rel_y);

        // Skip if sprite is behind player
        if (transform_y <= 0.1f) continue;

        // Calculate sprite screen position
        int sprite_screen_x = (int)((engine->screen_width / 2) * (1 + transform_x / transform_y));

        // Calculate sprite height and width
        int sprite_height = abs((int)(engine->screen_height / transform_y));
        int sprite_width = abs((int)(engine->screen_height / transform_y));

        // Calculate draw bounds
        int draw_start_y = -sprite_height / 2 + engine->screen_height / 2;
        if (draw_start_y < 0) draw_start_y = 0;
        int draw_end_y = sprite_height / 2 + engine->screen_height / 2;
        if (draw_end_y >= engine->screen_height) draw_end_y = engine->screen_height - 1;

        int draw_start_x = -sprite_width / 2 + sprite_screen_x;
        if (draw_start_x < 0) draw_start_x = 0;
        int draw_end_x = sprite_width / 2 + sprite_screen_x;
        if (draw_end_x >= engine->screen_width) draw_end_x = engine->screen_width - 1;

        // Get sprite texture based on type
        Texture* tex = NULL;
        uint32_t* tex_pixels = NULL;

        if (sprite_order[i].type == 1) {  // Enemy
            if (tex_id < 0 || tex_id >= em->texture_count) tex_id = 0;
            tex = &em->textures[tex_id];
            tex_pixels = tex->data;
        } else if (sprite_order[i].type == 2) {  // Pickup
            if (tex_id < 0 || tex_id >= PICKUP_COUNT) tex_id = 0;
            tex = &pm->textures[tex_id];
            tex_pixels = tex->data;
        } else {  // Static sprite
            if (tex_id < 0 || tex_id >= sm->texture_count) tex_id = 0;
            tex = &sm->sprite_textures[tex_id];
            tex_pixels = tex->data;
        }

        // Cache screen buffer pointer
        uint32_t* pixels = engine->pixels;

        // Pre-calculate sprite offset for tex_x calculation
        int sprite_offset = -sprite_width / 2 + sprite_screen_x;

        // Draw sprite
        for (int stripe = draw_start_x; stripe < draw_end_x; stripe++) {
            // Check if sprite is in front of wall (z-buffer)
            if (transform_y >= z_buffer[stripe]) continue;

            // Draw textured sprite (all types now use textures)
            int tex_x = (int)((stripe - sprite_offset) * TEXTURE_WIDTH / sprite_width);
            if (tex_x < 0 || tex_x >= TEXTURE_WIDTH) continue;

            for (int y = draw_start_y; y < draw_end_y; y++) {
                int d = y * 256 - engine->screen_height * 128 + sprite_height * 128;
                int tex_y = ((d * TEXTURE_HEIGHT) / sprite_height) / 256;

                if (tex_y < 0 || tex_y >= TEXTURE_HEIGHT) continue;

                uint32_t color = tex_pixels[tex_y * TEXTURE_WIDTH + tex_x];

                // Apply hit flash for enemies
                if (sprite_order[i].type == 1) {  // Enemy
                    Enemy* enemy = &em->enemies[sprite_order[i].sprite_index];
                    if (enemy->hit_flash_time > 0.0f) {
                        // Flash white when hit
                        float intensity = enemy->hit_flash_time / 0.15f;
                        uint32_t r = ((color >> 16) & 0xFF);
                        uint32_t g = ((color >> 8) & 0xFF);
                        uint32_t b = (color & 0xFF);

                        r = r + (255 - r) * intensity;
                        g = g + (255 - g) * intensity;
                        b = b + (255 - b) * intensity;

                        color = (color & 0xFF000000) | (r << 16) | (g << 8) | b;
                    }
                }

                // Check alpha (transparency) - only render if alpha > 128 (more than 50% opaque)
                uint8_t alpha = (color >> 24) & 0xFF;
                if (alpha > 128) {
                    pixels[y * engine->screen_width + stripe] = color & 0x00FFFFFF;
                }
            }
        }
    }
}
