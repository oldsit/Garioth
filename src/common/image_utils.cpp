#include "image_utils.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>
#include <string>
#include <iostream>

namespace ImageUtils {

    // Load an image from a file and return a pointer to the image data.
    unsigned char* loadImage(const std::string& path, int& width, int& height, int& channels) {
        // Load the image using stb_image
        unsigned char* image = stbi_load(path.c_str(), &width, &height, &channels, 0);
        if (!image) {
            std::cerr << "Error loading image: " << path << std::endl;
        }
        return image;
    }

    // Resize an image using stb_image_resize (this is a simple example, you might need to adjust it).
    unsigned char* resizeImage(unsigned char* image, int& width, int& height, int newWidth, int newHeight) {
        int newChannels = 4; // RGBA format
        unsigned char* resizedImage = (unsigned char*)malloc(newWidth * newHeight * newChannels);

        if (stbir_resize_uint8(image, width, height, 0, resizedImage, newWidth, newHeight, 0, newChannels)) {
            width = newWidth;
            height = newHeight;
            return resizedImage;
        } else {
            std::cerr << "Error resizing image." << std::endl;
            return nullptr;
        }
    }

    // Free image memory allocated by stb_image.
    void freeImage(unsigned char* image) {
        stbi_image_free(image);
    }

    // Save an image to a file using stb_image_write.
    bool saveImage(const std::string& path, unsigned char* image, int width, int height, int channels) {
        int result = stbi_write_png(path.c_str(), width, height, channels, image, width * channels);
        return result != 0;
    }
}
