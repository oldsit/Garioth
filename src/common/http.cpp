#include "http.h"
#include <curl/include/curl/curl.h>
#include <iostream>
#include <sstream>

// Utility function to send POST requests
bool HTTP::sendRequest(const std::string& url, const std::string& postData) {
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());

        // Set headers for JSON content type
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Request failed: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            return false;
        }

        curl_easy_cleanup(curl);
        return true;
    }

    return false;
}

// Function to convert JSONReader object to a JSON string
std::string HTTP::jsonToString(const JSONReader& jsonData) {
    // Convert the JSONReader object to a string manually, since we aren't using jsoncpp
    std::ostringstream oss;
    oss << "{";
    bool first = true;
    for (const auto& pair : jsonData.getData()) {  // Assuming `getData()` returns your internal map of JSON data
        if (!first) {
            oss << ",";
        }
        oss << "\"" << pair.first << "\":\"" << pair.second << "\"";  // You may need to adjust this for nested JSON
        first = false;
    }
    oss << "}";
    return oss.str();
}

// Function to send a login request
bool HTTP::loginUser(const std::string& username, const std::string& password) {
    std::string url = "http://localhost:3000/login";
    JSONReader jsonData;
    jsonData.add("username", username);
    jsonData.add("password", password);

    std::string postData = jsonToString(jsonData);
    return sendRequest(url, postData);
}

// Function to send a register request
bool HTTP::registerUser(const std::string& username, const std::string& email, const std::string& password) {
    std::string url = "http://localhost:3000/register";
    JSONReader jsonData;
    jsonData.add("username", username);
    jsonData.add("email", email);
    jsonData.add("password", password);

    std::string postData = jsonToString(jsonData);
    return sendRequest(url, postData);
}
