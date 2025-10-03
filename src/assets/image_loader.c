#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "image_loader.h"
#include <stdio.h>
#include <stdlib.h>

uint32_t* image_load(const char* filepath, int* width, int* height) {
    int channels;
    unsigned char* img_data = stbi_load(filepath, width, height, &channels, 4); // Force RGBA

    if (!img_data) {
        fprintf(stderr, "Failed to load image: %s\n", filepath);
        return NULL;
    }

    // Convert RGBA to ARGB8888
    int pixel_count = (*width) * (*height);
    uint32_t* pixels = (uint32_t*)malloc(pixel_count * sizeof(uint32_t));

    if (!pixels) {
        stbi_image_free(img_data);
        return NULL;
    }

    for (int i = 0; i < pixel_count; i++) {
        unsigned char r = img_data[i * 4 + 0];
        unsigned char g = img_data[i * 4 + 1];
        unsigned char b = img_data[i * 4 + 2];
        unsigned char a = img_data[i * 4 + 3];

        // Pack into ARGB8888
        pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }

    stbi_image_free(img_data);
    return pixels;
}

void image_free(uint32_t* data) {
    free(data);
}
