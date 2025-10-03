#include "raycaster.h"
#include "map.h"
#include "texture.h"
#include "sprite.h"
#include "enemy.h"
#include <math.h>
#include <string.h>

// Forward declaration
void render_sprites(Engine* engine, Player* player, SpriteManager* sm, EnemyManager* em, PickupManager* pm, float* z_buffer);

void raycaster_render(Engine* engine, Player* player, TextureManager* tm, SpriteManager* sm, EnemyManager* em, PickupManager* pm) {
    // Allocate z-buffer dynamically based on current screen width
    static float* z_buffer = NULL;
    static int z_buffer_size = 0;

    if (z_buffer_size != engine->screen_width) {
        free(z_buffer);
        z_buffer = (float*)malloc(engine->screen_width * sizeof(float));
        z_buffer_size = engine->screen_width;
    }

    // Clear screen (floor and ceiling) - optimized
    uint32_t* pixels = engine->pixels;
    int half_screen = engine->screen_height / 2 * engine->screen_width;

    // Fill ceiling
    for (int i = 0; i < half_screen; i++) {
        pixels[i] = 0x333333;
    }

    // Fill floor
    for (int i = half_screen; i < engine->screen_width * engine->screen_height; i++) {
        pixels[i] = 0x666666;
    }

    // Cast rays
    for (int x = 0; x < engine->screen_width; x++) {
        // Calculate ray position and direction
        float camera_x = 2 * x / (float)engine->screen_width - 1;
        float ray_dir_x = player->dir_x + player->plane_x * camera_x;
        float ray_dir_y = player->dir_y + player->plane_y * camera_x;

        // Which box of the map we're in
        int map_x = (int)player->x;
        int map_y = (int)player->y;

        // Length of ray from current position to next x or y-side
        float side_dist_x;
        float side_dist_y;

        // Length of ray from one x or y-side to next x or y-side
        float delta_dist_x = (ray_dir_x == 0) ? 1e30 : fabsf(1 / ray_dir_x);
        float delta_dist_y = (ray_dir_y == 0) ? 1e30 : fabsf(1 / ray_dir_y);
        float perp_wall_dist;

        // What direction to step in x or y-direction (either +1 or -1)
        int step_x;
        int step_y;

        int hit = 0;  // Was there a wall hit?
        int side;     // Was a NS or a EW wall hit?

        // Calculate step and initial sideDist
        if (ray_dir_x < 0) {
            step_x = -1;
            side_dist_x = (player->x - map_x) * delta_dist_x;
        } else {
            step_x = 1;
            side_dist_x = (map_x + 1.0 - player->x) * delta_dist_x;
        }
        if (ray_dir_y < 0) {
            step_y = -1;
            side_dist_y = (player->y - map_y) * delta_dist_y;
        } else {
            step_y = 1;
            side_dist_y = (map_y + 1.0 - player->y) * delta_dist_y;
        }

        // Perform DDA
        while (hit == 0) {
            // Jump to next map square, either in x-direction, or in y-direction
            if (side_dist_x < side_dist_y) {
                side_dist_x += delta_dist_x;
                map_x += step_x;
                side = 0;
            } else {
                side_dist_y += delta_dist_y;
                map_y += step_y;
                side = 1;
            }
            // Check if ray has hit a wall
            if (world_map[map_x][map_y] > 0) hit = 1;
        }

        // Calculate distance projected on camera direction
        if (side == 0) {
            perp_wall_dist = (map_x - player->x + (1 - step_x) / 2) / ray_dir_x;
        } else {
            perp_wall_dist = (map_y - player->y + (1 - step_y) / 2) / ray_dir_y;
        }

        // Store perpendicular distance in z-buffer for sprite rendering
        z_buffer[x] = perp_wall_dist;

        // Calculate height of line to draw on screen
        int line_height = (int)(engine->screen_height / perp_wall_dist);

        // Calculate lowest and highest pixel to fill in current stripe
        int draw_start = -line_height / 2 + engine->screen_height / 2;
        if (draw_start < 0) draw_start = 0;
        int draw_end = line_height / 2 + engine->screen_height / 2;
        if (draw_end >= engine->screen_height) draw_end = engine->screen_height - 1;

        // Get texture for this wall
        int tex_num = world_map[map_x][map_y] - 1;
        if (tex_num < 0) tex_num = 0;
        if (tex_num >= MAX_TEXTURES) tex_num = MAX_TEXTURES - 1;

        // Calculate where exactly the wall was hit
        float wall_x;
        if (side == 0) {
            wall_x = player->y + perp_wall_dist * ray_dir_y;
        } else {
            wall_x = player->x + perp_wall_dist * ray_dir_x;
        }
        wall_x -= floorf(wall_x);

        // X coordinate on the texture
        int tex_x = (int)(wall_x * (float)TEXTURE_WIDTH);
        if (side == 0 && ray_dir_x > 0) tex_x = TEXTURE_WIDTH - tex_x - 1;
        if (side == 1 && ray_dir_y < 0) tex_x = TEXTURE_WIDTH - tex_x - 1;

        // Calculate step size for texture mapping
        float step = 1.0 * TEXTURE_HEIGHT / line_height;
        float tex_pos = (draw_start - engine->screen_height / 2 + line_height / 2) * step;

        // Cache texture pointer for faster access
        Texture* tex = &tm->textures[tex_num];
        uint32_t* tex_pixels = tex->data;

        // Draw the textured vertical line
        for (int y = draw_start; y < draw_end; y++) {
            int tex_y = (int)tex_pos & (TEXTURE_HEIGHT - 1);
            tex_pos += step;

            uint32_t color = tex_pixels[tex_y * TEXTURE_WIDTH + tex_x];

            // Give x and y sides different brightness
            if (side == 1) {
                color = (color >> 1) & 0x7F7F7F;
            }

            pixels[y * engine->screen_width + x] = color;
        }
    }

    // Render sprites after walls
    if (sm) {
        render_sprites(engine, player, sm, em, pm, z_buffer);
    }
}
