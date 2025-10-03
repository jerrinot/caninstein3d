#include "hud.h"
#include <stdio.h>

void hud_draw_pixel(Engine* engine, int x, int y, uint32_t color) {
    if (x < 0 || x >= engine->screen_width || y < 0 || y >= engine->screen_height) {
        return;
    }
    engine->pixels[y * engine->screen_width + x] = color;
}

void hud_draw_rect(Engine* engine, int x, int y, int width, int height, uint32_t color) {
    for (int dy = 0; dy < height; dy++) {
        for (int dx = 0; dx < width; dx++) {
            hud_draw_pixel(engine, x + dx, y + dy, color);
        }
    }
}

// Simple 3x5 bitmap font for numbers
static const uint8_t font_numbers[10][5] = {
    {0b111, 0b101, 0b101, 0b101, 0b111}, // 0
    {0b010, 0b110, 0b010, 0b010, 0b111}, // 1
    {0b111, 0b001, 0b111, 0b100, 0b111}, // 2
    {0b111, 0b001, 0b111, 0b001, 0b111}, // 3
    {0b101, 0b101, 0b111, 0b001, 0b001}, // 4
    {0b111, 0b100, 0b111, 0b001, 0b111}, // 5
    {0b111, 0b100, 0b111, 0b101, 0b111}, // 6
    {0b111, 0b001, 0b010, 0b010, 0b010}, // 7
    {0b111, 0b101, 0b111, 0b101, 0b111}, // 8
    {0b111, 0b101, 0b111, 0b001, 0b111}, // 9
};

void hud_draw_number(Engine* engine, int x, int y, int number, uint32_t color) {
    if (number == -1) {
        // Draw infinity symbol (simplified as "INF")
        // For now, just skip
        return;
    }

    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%d", number);

    int cursor_x = x;
    for (int i = 0; buffer[i] != '\0'; i++) {
        if (buffer[i] >= '0' && buffer[i] <= '9') {
            int digit = buffer[i] - '0';
            // Draw digit using bitmap font
            for (int row = 0; row < 5; row++) {
                for (int col = 0; col < 3; col++) {
                    if (font_numbers[digit][row] & (1 << (2 - col))) {
                        hud_draw_pixel(engine, cursor_x + col * 2, y + row * 2, color);
                        hud_draw_pixel(engine, cursor_x + col * 2 + 1, y + row * 2, color);
                        hud_draw_pixel(engine, cursor_x + col * 2, y + row * 2 + 1, color);
                        hud_draw_pixel(engine, cursor_x + col * 2 + 1, y + row * 2 + 1, color);
                    }
                }
            }
            cursor_x += 8;  // 3*2 + 2 spacing
        }
    }
}

void hud_draw_crosshair(Engine* engine) {
    int center_x = engine->screen_width / 2;
    int center_y = engine->screen_height / 2;
    uint32_t color = 0xFFFFFF;  // White

    // Draw cross
    int size = 5;
    for (int i = -size; i <= size; i++) {
        hud_draw_pixel(engine, center_x + i, center_y, color);
        hud_draw_pixel(engine, center_x, center_y + i, color);
    }

    // Draw center dot
    hud_draw_pixel(engine, center_x, center_y, 0xFF0000);  // Red dot
}

void hud_draw_health(Engine* engine, Player* player) {
    // Health in bottom-left area
    int hud_bottom = engine->screen_height - 80;
    int x = 10;
    int y = hud_bottom + 10;

    // Draw "HEALTH:" label
    hud_draw_rect(engine, x, y, 70, 12, 0x880000);  // Dark red background

    // Draw large health number (4x scale fits better in 80px HUD)
    int digit_scale = 4;
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%d", player->health);

    int cursor_x = x + 5;
    int number_y = y + 20;  // 12px label + 8px gap
    for (int i = 0; buffer[i] != '\0'; i++) {
        if (buffer[i] >= '0' && buffer[i] <= '9') {
            int digit = buffer[i] - '0';
            for (int row = 0; row < 5; row++) {
                for (int col = 0; col < 3; col++) {
                    if (font_numbers[digit][row] & (1 << (2 - col))) {
                        // Draw bigger pixels
                        for (int dy = 0; dy < digit_scale; dy++) {
                            for (int dx = 0; dx < digit_scale; dx++) {
                                hud_draw_pixel(engine,
                                    cursor_x + col * digit_scale * 2 + dx,
                                    number_y + row * digit_scale * 2 + dy,
                                    0xFF0000);  // Bright red
                            }
                        }
                    }
                }
            }
            cursor_x += 10 * digit_scale;  // Tighter spacing
        }
    }
}

void hud_draw_ammo(Engine* engine, Player* player) {
    Weapon* weapon = player_get_current_weapon(player);

    // Ammo in bottom-right area
    int hud_bottom = engine->screen_height - 80;
    int x = engine->screen_width - 150;
    int y = hud_bottom + 10;

    // Draw "AMMO:" label
    hud_draw_rect(engine, x, y, 60, 12, 0x888800);  // Dark yellow background

    // Only show ammo for weapons that use it
    if (weapon->max_ammo == -1) {
        // Draw "INF" for infinite
        hud_draw_rect(engine, x + 10, y + 20, 10, 35, 0xFFFF00);
        return;
    }

    // Draw large ammo number (4x scale)
    int digit_scale = 4;
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%d", weapon->ammo);

    int cursor_x = x + 5;
    int number_y = y + 20;  // 12px label + 8px gap
    for (int i = 0; buffer[i] != '\0'; i++) {
        if (buffer[i] >= '0' && buffer[i] <= '9') {
            int digit = buffer[i] - '0';
            for (int row = 0; row < 5; row++) {
                for (int col = 0; col < 3; col++) {
                    if (font_numbers[digit][row] & (1 << (2 - col))) {
                        for (int dy = 0; dy < digit_scale; dy++) {
                            for (int dx = 0; dx < digit_scale; dx++) {
                                hud_draw_pixel(engine,
                                    cursor_x + col * digit_scale * 2 + dx,
                                    number_y + row * digit_scale * 2 + dy,
                                    0xFFFF00);  // Bright yellow
                            }
                        }
                    }
                }
            }
            cursor_x += 10 * digit_scale;  // Tighter spacing
        }
    }
}

void hud_draw_weapon_name(Engine* engine, Player* player) {
    Weapon* weapon = player_get_current_weapon(player);

    // Draw weapon display in bottom-center
    int hud_bottom = engine->screen_height - 80;
    int center_x = engine->screen_width / 2;
    int y = hud_bottom;

    // Draw weapon box (large)
    int box_width = 120;
    int box_height = 80;
    int box_x = center_x - box_width / 2;

    // Background
    hud_draw_rect(engine, box_x, y, box_width, box_height, 0x333333);

    // Border
    hud_draw_rect(engine, box_x, y, box_width, 3, 0x888888);
    hud_draw_rect(engine, box_x, y + box_height - 3, box_width, 3, 0x888888);
    hud_draw_rect(engine, box_x, y, 3, box_height, 0x888888);
    hud_draw_rect(engine, box_x + box_width - 3, y, 3, box_height, 0x888888);

    // Draw weapon graphic (simplified - different shapes for each weapon)
    int weapon_y = y + 20;
    int weapon_x = center_x;

    switch (weapon->type) {
        case WEAPON_KNIFE:
            // Draw knife (diagonal line)
            for (int i = 0; i < 25; i++) {
                hud_draw_rect(engine, weapon_x - 10 + i, weapon_y + i, 3, 3, 0xCCCCCC);
            }
            break;

        case WEAPON_PISTOL:
            // Draw pistol (L-shape)
            hud_draw_rect(engine, weapon_x - 5, weapon_y + 10, 10, 20, 0x555555);
            hud_draw_rect(engine, weapon_x - 15, weapon_y + 10, 25, 8, 0x555555);
            hud_draw_rect(engine, weapon_x - 15, weapon_y + 5, 8, 8, 0x777777);
            break;

        case WEAPON_SHOTGUN:
            // Draw shotgun (long rectangle)
            hud_draw_rect(engine, weapon_x - 25, weapon_y + 15, 50, 10, 0x8B4513);
            hud_draw_rect(engine, weapon_x + 15, weapon_y + 10, 12, 20, 0x654321);
            hud_draw_rect(engine, weapon_x - 30, weapon_y + 12, 5, 16, 0x333333);
            break;

        case WEAPON_MACHINEGUN:
            // Draw machinegun (box with barrel)
            hud_draw_rect(engine, weapon_x - 20, weapon_y + 12, 40, 15, 0x2F4F4F);
            hud_draw_rect(engine, weapon_x - 30, weapon_y + 15, 15, 8, 0x1C1C1C);
            hud_draw_rect(engine, weapon_x + 10, weapon_y + 8, 5, 25, 0x444444);
            break;
    }

    // Draw weapon name below
    const char* names[] = {"KNIFE", "PISTOL", "SHOTGUN", "M-GUN"};
    // Simplified - just draw the weapon number large
    int digit_scale = 2;
    int digit = player->current_weapon_index + 1;
    int num_x = center_x - 6;
    int num_y = y + box_height - 25;

    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 3; col++) {
            if (font_numbers[digit][row] & (1 << (2 - col))) {
                for (int dy = 0; dy < digit_scale; dy++) {
                    for (int dx = 0; dx < digit_scale; dx++) {
                        hud_draw_pixel(engine,
                            num_x + col * digit_scale * 2 + dx,
                            num_y + row * digit_scale * 2 + dy,
                            0xFFFFFF);
                    }
                }
            }
        }
    }
}

void hud_draw_game_over(Engine* engine) {
    // Draw semi-transparent red overlay
    for (int y = 0; y < engine->screen_height; y++) {
        for (int x = 0; x < engine->screen_width; x++) {
            uint32_t current = engine->pixels[y * engine->screen_width + x];
            // Blend with red
            uint32_t r = ((current >> 16) & 0xFF) / 2 + 64;
            uint32_t g = ((current >> 8) & 0xFF) / 2;
            uint32_t b = (current & 0xFF) / 2;
            engine->pixels[y * engine->screen_width + x] = (r << 16) | (g << 8) | b;
        }
    }

    // Draw "GAME OVER" text (simplified - just draw large rectangles)
    int center_x = engine->screen_width / 2;
    int center_y = engine->screen_height / 2;

    // Background rectangle
    hud_draw_rect(engine, center_x - 120, center_y - 60, 240, 120, 0x000000);

    // Border
    hud_draw_rect(engine, center_x - 125, center_y - 65, 250, 5, 0xFF0000);
    hud_draw_rect(engine, center_x - 125, center_y + 60, 250, 5, 0xFF0000);
    hud_draw_rect(engine, center_x - 125, center_y - 60, 5, 120, 0xFF0000);
    hud_draw_rect(engine, center_x + 120, center_y - 60, 5, 120, 0xFF0000);

    // Draw "GAME OVER" text
    hud_draw_rect(engine, center_x - 80, center_y - 40, 160, 25, 0xFF0000);

    // Draw "Press R to Restart"
    hud_draw_rect(engine, center_x - 70, center_y + 10, 140, 15, 0x666666);
}

void hud_render(Engine* engine, Player* player) {
    // Draw bottom HUD panel (Doom/Wolf style)
    int hud_height = 80;
    int hud_y = engine->screen_height - hud_height;

    // Draw HUD background
    hud_draw_rect(engine, 0, hud_y, engine->screen_width, hud_height, 0x222222);

    // Draw top border line
    hud_draw_rect(engine, 0, hud_y, engine->screen_width, 2, 0x888888);

    // Draw HUD elements
    hud_draw_crosshair(engine);
    hud_draw_weapon_name(engine, player);  // Center (weapon display)
    hud_draw_health(engine, player);        // Left
    hud_draw_ammo(engine, player);          // Right

    // Draw kill counter and score - positioned at bottom of HUD
    int stats_y = hud_y + 58;  // Start at 58: label 8px + gap 2px + number 20px = 30px total, ends at 88 (within 80)

    // "KILLS:" label - left side, below health
    int kills_x = 120;  // Positioned to not overlap weapon box
    hud_draw_rect(engine, kills_x, stats_y, 50, 8, 0x444400);  // Dark bg

    // Draw kills number - smaller scale (2x) to fit in remaining space
    int digit_scale = 2;
    char kills_buffer[16];
    snprintf(kills_buffer, sizeof(kills_buffer), "%d", player->kills);
    int kills_num_x = kills_x + 52;
    for (int i = 0; kills_buffer[i] != '\0'; i++) {
        if (kills_buffer[i] >= '0' && kills_buffer[i] <= '9') {
            int digit = kills_buffer[i] - '0';
            for (int row = 0; row < 5; row++) {
                for (int col = 0; col < 3; col++) {
                    if (font_numbers[digit][row] & (1 << (2 - col))) {
                        for (int dy = 0; dy < digit_scale; dy++) {
                            for (int dx = 0; dx < digit_scale; dx++) {
                                hud_draw_pixel(engine,
                                    kills_num_x + col * digit_scale * 2 + dx,
                                    stats_y + 2 + row * digit_scale * 2 + dy,
                                    0xFFFF00);
                            }
                        }
                    }
                }
            }
            kills_num_x += 7 * digit_scale;
        }
    }

    // "SCORE:" label - right side, below ammo
    int score_x = engine->screen_width - 200;  // Positioned to not overlap weapon box
    hud_draw_rect(engine, score_x, stats_y, 55, 8, 0x004400);  // Dark green bg

    // Draw score number - smaller scale (2x) to fit
    char score_buffer[16];
    snprintf(score_buffer, sizeof(score_buffer), "%d", player->score);
    int score_num_x = score_x + 57;
    for (int i = 0; score_buffer[i] != '\0'; i++) {
        if (score_buffer[i] >= '0' && score_buffer[i] <= '9') {
            int digit = score_buffer[i] - '0';
            for (int row = 0; row < 5; row++) {
                for (int col = 0; col < 3; col++) {
                    if (font_numbers[digit][row] & (1 << (2 - col))) {
                        for (int dy = 0; dy < digit_scale; dy++) {
                            for (int dx = 0; dx < digit_scale; dx++) {
                                hud_draw_pixel(engine,
                                    score_num_x + col * digit_scale * 2 + dx,
                                    stats_y + 2 + row * digit_scale * 2 + dy,
                                    0x00FF00);
                            }
                        }
                    }
                }
            }
            score_num_x += 7 * digit_scale;
        }
    }

    // Draw game over if player is dead
    if (!player_is_alive(player)) {
        hud_draw_game_over(engine);
    }
}
