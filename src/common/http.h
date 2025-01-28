#ifndef HTTP_H
#define HTTP_H

#include <string>
#include "json_reader.h"  // Include your custom JSONReader header

class HTTP {
public:
    static bool sendPostRequest(const std::string& url, const JSONReader& jsonData);
    static bool loginUser(const std::string& username, const std::string& password); // Removed ipAddress parameter
    static bool registerUser(const std::string& username, const std::string& email, const std::string& password);

private:
    static bool sendRequest(const std::string& url, const std::string& postData);
    static std::string jsonToString(const JSONReader& jsonData);
};

#endif // HTTP_H
