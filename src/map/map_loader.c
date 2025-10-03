#include "map.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

bool map_load(Map* map, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open map file: %s\n", filename);
        return false;
    }

    char line[512];
    int row = 0;
    bool reading_data = false;
    bool size_read = false;

    // Initialize defaults
    map->width = MAP_WIDTH;
    map->height = MAP_HEIGHT;
    map->player_spawn_x = 12.0f;
    map->player_spawn_y = 12.0f;
    memset(map->data, 0, sizeof(map->data));

    while (fgets(line, sizeof(line), file)) {
        // Skip comments and empty lines
        char* ptr = line;
        while (*ptr && isspace(*ptr)) ptr++;
        if (*ptr == '#' || *ptr == '\0' || *ptr == '\n') {
            continue;
        }

        // Read map size (first non-comment line)
        if (!size_read) {
            if (sscanf(line, "%d %d", &map->width, &map->height) == 2) {
                if (map->width > MAP_WIDTH) map->width = MAP_WIDTH;
                if (map->height > MAP_HEIGHT) map->height = MAP_HEIGHT;
                size_read = true;
                reading_data = true;
                continue;
            }
        }

        // Try to read player spawn position
        float px, py;
        if (sscanf(line, "%f %f", &px, &py) == 2 && row >= map->height) {
            map->player_spawn_x = px;
            map->player_spawn_y = py;
            continue;
        }

        // Read map data
        if (reading_data && row < map->height) {
            char* token = strtok(line, " \t\n");
            int col = 0;

            while (token && col < map->width) {
                map->data[row][col] = atoi(token);
                token = strtok(NULL, " \t\n");
                col++;
            }
            row++;
        }
    }

    fclose(file);

    // Copy to global world_map for compatibility
    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            world_map[y][x] = map->data[y][x];
        }
    }

    printf("Map loaded: %dx%d, spawn at (%.1f, %.1f)\n",
           map->width, map->height, map->player_spawn_x, map->player_spawn_y);

    return true;
}

void map_free(Map* map) {
    // Nothing to free for now, but keep for future use
    (void)map;
}
