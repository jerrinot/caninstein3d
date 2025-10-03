#ifndef MAP_H
#define MAP_H

#include <stdbool.h>

#define MAP_WIDTH 24
#define MAP_HEIGHT 24

typedef struct {
    int width;
    int height;
    int data[MAP_WIDTH][MAP_HEIGHT];
    float player_spawn_x;
    float player_spawn_y;
} Map;

// Map data (legacy)
extern int world_map[MAP_WIDTH][MAP_HEIGHT];

// Map functions
bool map_load(Map* map, const char* filename);
void map_free(Map* map);

#endif
