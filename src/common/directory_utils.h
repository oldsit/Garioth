#ifndef DIRECTORY_UTILS_H
#define DIRECTORY_UTILS_H

#include <string>
#include <filesystem>

namespace DirectoryUtils {

    // Create a directory if it doesn't exist
    bool createDirectory(const std::string& path);

    // Check if a directory exists
    bool directoryExists(const std::string& path);

    // Get the current working directory
    std::string getCurrentWorkingDirectory();

}

#endif // DIRECTORY_UTILS_H
