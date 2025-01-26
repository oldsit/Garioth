#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_image_resize.h"

// Function declarations for image ops
unsigned char* loadImage(const char* filepath, int width, int* height, int* channels);
void saveImage(const char* filepath, int width, int height, int channels, unsigned char* data);
void freeImage(unsigned char* imageData);
unsigned char* resizeImage(unsigned char* inputImage, int inputWidth, int inputHeight, int channels, int outputWidth, int outputHeight);

#endif

