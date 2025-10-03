#include "minimap.h"
#include "sprite.h"
#include <math.h>

void minimap_render(Engine* engine, Player* player, Map* map, bool enabled) {
    if (!enabled) {
        return;
    }

    int minimap_size = map->width * MINIMAP_SCALE;
    int minimap_x = MINIMAP_MARGIN;
    int minimap_y = MINIMAP_MARGIN;

    // Draw map tiles
    // Note: world_map is indexed as [x][y] in the raycaster
    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            int screen_x = minimap_x + y * MINIMAP_SCALE;  // Swapped: use y for x
            int screen_y = minimap_y + x * MINIMAP_SCALE;  // Swapped: use x for y

            uint32_t color;
            if (map->data[x][y] > 0) {  // Swapped indices
                // Wall - white
                color = 0xCCCCCC;
            } else {
                // Empty - dark gray
                color = 0x333333;
            }

            // Draw tile
            for (int dy = 0; dy < MINIMAP_SCALE; dy++) {
                for (int dx = 0; dx < MINIMAP_SCALE; dx++) {
                    int px = screen_x + dx;
                    int py = screen_y + dy;
                    if (px >= 0 && px < engine->screen_width && py >= 0 && py < engine->screen_height) {
                        engine->pixels[py * engine->screen_width + px] = color;
                    }
                }
            }
        }
    }

    // Draw player position (swap x and y to match map orientation)
    int player_screen_x = minimap_x + (int)(player->y * MINIMAP_SCALE);
    int player_screen_y = minimap_y + (int)(player->x * MINIMAP_SCALE);

    // Draw player as red dot
    for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++) {
            int px = player_screen_x + dx;
            int py = player_screen_y + dy;
            if (px >= 0 && px < engine->screen_width && py >= 0 && py < engine->screen_height) {
                engine->pixels[py * engine->screen_width + px] = 0xFF0000;
            }
        }
    }

    // Draw player direction line (swap dir_x and dir_y to match map orientation)
    for (int i = 0; i < 15; i++) {
        int px = player_screen_x + (int)(player->dir_y * i * MINIMAP_SCALE / 2);
        int py = player_screen_y + (int)(player->dir_x * i * MINIMAP_SCALE / 2);
        if (px >= 0 && px < engine->screen_width && py >= 0 && py < engine->screen_height) {
            engine->pixels[py * engine->screen_width + px] = 0xFFFF00;
        }
    }

    // Draw border
    uint32_t border_color = 0xFFFFFF;
    for (int i = 0; i < minimap_size; i++) {
        // Top border
        engine->pixels[minimap_y * engine->screen_width + (minimap_x + i)] = border_color;
        // Bottom border
        engine->pixels[(minimap_y + minimap_size - 1) * engine->screen_width + (minimap_x + i)] = border_color;
        // Left border
        engine->pixels[(minimap_y + i) * engine->screen_width + minimap_x] = border_color;
        // Right border
        engine->pixels[(minimap_y + i) * engine->screen_width + (minimap_x + minimap_size - 1)] = border_color;
    }
}
