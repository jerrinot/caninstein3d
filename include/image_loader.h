#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

#include <stdint.h>
#include <stdbool.h>

// Load image from file (PNG, JPG, BMP supported)
// Returns pixel data in ARGB8888 format
// Caller must free the returned data with image_free()
uint32_t* image_load(const char* filepath, int* width, int* height);

// Free image data
void image_free(uint32_t* data);

#endif
