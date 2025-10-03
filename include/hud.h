#ifndef HUD_H
#define HUD_H

#include "engine.h"
#include "player.h"

// Render HUD elements
void hud_render(Engine* engine, Player* player);

// Draw crosshair at screen center
void hud_draw_crosshair(Engine* engine);

// Draw health bar
void hud_draw_health(Engine* engine, Player* player);

// Draw ammo counter
void hud_draw_ammo(Engine* engine, Player* player);

// Draw weapon name
void hud_draw_weapon_name(Engine* engine, Player* player);

// Draw game over screen
void hud_draw_game_over(Engine* engine);

// Helper function to draw a pixel
void hud_draw_pixel(Engine* engine, int x, int y, uint32_t color);

// Helper function to draw a filled rectangle
void hud_draw_rect(Engine* engine, int x, int y, int width, int height, uint32_t color);

// Helper function to draw simple text (numbers only)
void hud_draw_number(Engine* engine, int x, int y, int number, uint32_t color);

#endif
