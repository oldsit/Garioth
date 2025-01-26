#include "directory_utils.h"
#include <iostream>
#include <filesystem>

namespace DirectoryUtils {

    // Create a directory if it doesn't exist
    bool createDirectory(const std::string& path) {
        try {
            if (!std::filesystem::exists(path)) {
                std::filesystem::create_directory(path);
                return true;
            }
            return false;
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error creating directory: " << e.what() << std::endl;
            return false;
        }
    }

    // Check if a directory exists
    bool directoryExists(const std::string& path) {
        return std::filesystem::exists(path);
    }

    // Get the current working directory
    std::string getCurrentWorkingDirectory() {
        return std::filesystem::current_path().string();
    }
}
