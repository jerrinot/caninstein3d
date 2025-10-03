#ifndef INPUT_H
#define INPUT_H

#include "player.h"
#include "engine.h"
#include "sound.h"
#include <stdbool.h>

// Input state
typedef struct {
    bool shoot_pressed;      // Mouse/space pressed this frame
    int weapon_switch;       // Weapon number (1-4) or -1 for no switch
} InputState;

void input_handle(Player* player, float delta_time, InputState* input_state, SoundManager* sm);
void input_handle_mouse(Player* player, Engine* engine);

#endif
