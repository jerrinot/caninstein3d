#ifndef RAYCASTER_H
#define RAYCASTER_H

#include "engine.h"
#include "player.h"
#include "texture.h"
#include "sprite.h"
#include "enemy.h"
#include "pickup.h"

void raycaster_render(Engine* engine, Player* player, TextureManager* tm, SpriteManager* sm, EnemyManager* em, PickupManager* pm);

#endif
