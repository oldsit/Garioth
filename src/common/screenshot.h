#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stb_image_write.h>
#include <string>

void SaveScreenshot(const std::string& filename, GLFWwindow* window);

#endif