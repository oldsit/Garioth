#include "screenshot.h"
#include <iostream>
#include <vector>

void SaveScreenshot(const std::string& filename, GLFWwindow* window) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    std::vector<unsigned char> pixels(3 * width * height);  // 3 channels (RGB)

    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Flip image vertically
    for (int y = 0; y < height / 2; ++y) {
        for (int x = 0; x < width; ++x) {
            std::swap(pixels[3 * (y * width + x)], pixels[3 * ((height - 1 - y) * width + x)]);
            std::swap(pixels[3 * (y * width + x) + 1], pixels[3 * ((height - 1 - y) * width + x) + 1]);
            std::swap(pixels[3 * (y * width + x) + 2], pixels[3 * ((height - 1 - y) * width + x) + 2]);
        }
    }

    if (stbi_write_png(filename.c_str(), width, height, 3, pixels.data(), width * 3)) {
        std::cout << "Screenshot saved to " << filename << std::endl;
    } else {
        std::cout << "Failed to save screenshot." << std::endl;
    }
}