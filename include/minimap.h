#ifndef MINIMAP_H
#define MINIMAP_H

#include "engine.h"
#include "player.h"
#include "map.h"
#include <stdbool.h>

#define MINIMAP_SCALE 8
#define MINIMAP_MARGIN 10

void minimap_render(Engine* engine, Player* player, Map* map, bool enabled);

#endif
