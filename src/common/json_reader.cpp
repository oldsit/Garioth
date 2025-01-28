#include "json_reader.h"
#include <iostream>
#include <algorithm>
#include <sstream>

// Remove whitespaces, newlines, and tabs
void JSONReader::removeWhitespace(std::string& str) {
    str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end());
}

// Parse the input JSON string
bool JSONReader::parse(const std::string& input) {
    std::string str = input;
    // Remove spaces, newlines, and tabs
    removeWhitespace(str);

    // Ensure the input starts and ends with braces
    if (str.front() != '{' || str.back() != '}') {
        std::cerr << "Invalid JSON format: Must start with '{' and end with '}'." << std::endl;
        return false;
    }

    str = str.substr(1, str.size() - 2);  // Remove the outer braces
    std::istringstream stream(str);
    std::string key, value;

    while (getQuotedString(stream, key)) {  // Read the key (with quotes)
        if (!(stream >> std::ws && stream.get() == ':')) {  // Ensure a colon separates key and value
            std::cerr << "Invalid format: Key-value pair should be separated by ':'." << std::endl;
            return false;
        }

        if (!parseValue(stream, value)) {  // Parse the value
            std::cerr << "Invalid format: Value should be a valid JSON type." << std::endl;
            return false;
        }

        // Insert the key-value pair into the map
        jsonData[key] = std::make_shared<JSONReader>();  // Support for nested JSON
        jsonData[key]->parse(value);

        // If there's a comma, skip it and continue reading the next pair
        char nextChar;
        if (stream >> nextChar && nextChar == ',') {
            continue;
        }
        break;
    }

    return true;
}

// Helper function to parse a value (string, number, or nested object)
bool JSONReader::parseValue(std::istringstream& stream, std::string& value) {
    char nextChar = stream.peek();
    if (nextChar == '"') {
        return getQuotedString(stream, value);
    } else if (nextChar == '{') {
        return parseObject(stream);  // Handle nested object
    }
    return false;
}

// Helper function to parse a nested JSON object
bool JSONReader::parseObject(std::istringstream& stream) {
    char nextChar;
    stream >> nextChar;  // Skip '{'
    std::string key, value;
    while (getQuotedString(stream, key)) {
        if (!(stream >> std::ws && stream.get() == ':')) {
            std::cerr << "Invalid format: Key-value pair should be separated by ':'." << std::endl;
            return false;
        }
        if (!parseValue(stream, value)) {
            std::cerr << "Invalid format: Value should be a valid JSON type." << std::endl;
            return false;
        }
        jsonData[key] = std::make_shared<JSONReader>();
        jsonData[key]->parse(value);
        char nextChar;
    }
}

bool JSONReader::getQuotedString(std::istringstream& stream, std::string& str) {
    char nextChar;
    // Check if the next character is a double quote (")
    if (!(stream >> nextChar) || nextChar != '"') {
        return false;  // Invalid format, expected a starting quote
    }

    // Read characters until a closing quote is encountered
    std::ostringstream oss;
    while (stream.get(nextChar) && nextChar != '"') {
        oss.put(nextChar);
    }

    if (nextChar != '"') {
        return false;  // Invalid format, expected a closing quote
    }

    str = oss.str();  // Store the result in the reference string
    return true;
}