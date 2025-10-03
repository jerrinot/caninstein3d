#include "engine.h"
#include "player.h"
#include "input.h"
#include <SDL2/SDL.h>
#include <math.h>

void input_handle(Player* player, float delta_time, InputState* input_state, SoundManager* sm) {
    const uint8_t* keys = SDL_GetKeyboardState(NULL);

    // Reset input state
    input_state->shoot_pressed = false;
    input_state->weapon_switch = -1;

    // Movement
    if (keys[SDL_SCANCODE_W]) {
        player_move_forward(player, delta_time, sm);
    }
    if (keys[SDL_SCANCODE_S]) {
        player_move_backward(player, delta_time, sm);
    }
    if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) {
        player_rotate_left(player, delta_time);
    }
    if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) {
        player_rotate_right(player, delta_time);
    }

    // Shooting (spacebar)
    if (keys[SDL_SCANCODE_SPACE]) {
        input_state->shoot_pressed = true;
    }

    // Weapon switching (1-4 keys)
    if (keys[SDL_SCANCODE_1]) {
        input_state->weapon_switch = 0;  // WEAPON_KNIFE
    }
    if (keys[SDL_SCANCODE_2]) {
        input_state->weapon_switch = 1;  // WEAPON_PISTOL
    }
    if (keys[SDL_SCANCODE_3]) {
        input_state->weapon_switch = 2;  // WEAPON_SHOTGUN
    }
    if (keys[SDL_SCANCODE_4]) {
        input_state->weapon_switch = 3;  // WEAPON_MACHINEGUN
    }

    // Mouse click for shooting
    uint32_t mouse_state = SDL_GetMouseState(NULL, NULL);
    if (mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        input_state->shoot_pressed = true;
    }
}

void input_handle_mouse(Player* player, Engine* engine) {
    if (!engine->mouse_captured) {
        return;
    }

    int mouse_x, mouse_y;
    SDL_GetRelativeMouseState(&mouse_x, &mouse_y);

    if (mouse_x != 0) {
        // Calculate rotation amount based on mouse movement (negate for correct direction)
        float rot_amount = -(float)mouse_x * engine->mouse_sensitivity / 5000.0f;

        // Rotate player direction
        float old_dir_x = player->dir_x;
        player->dir_x = player->dir_x * cosf(rot_amount) - player->dir_y * sinf(rot_amount);
        player->dir_y = old_dir_x * sinf(rot_amount) + player->dir_y * cosf(rot_amount);

        // Rotate camera plane
        float old_plane_x = player->plane_x;
        player->plane_x = player->plane_x * cosf(rot_amount) - player->plane_y * sinf(rot_amount);
        player->plane_y = old_plane_x * sinf(rot_amount) + player->plane_y * cosf(rot_amount);
    }
}
