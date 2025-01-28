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
        jsonData[key] = value;  // Use the value directly for simple types (string or number)

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
        return getQuotedString(stream, value);  // String value
    } else if (nextChar == '{') {
        return parseObject(stream, value);  // Handle nested object
    } else {
        // Parse numbers or other types (for simplicity, assuming only strings and numbers are supported)
        return parseNumberOrBoolean(stream, value);
    }
}

// Helper function to parse a nested JSON object
bool JSONReader::parseObject(std::istringstream& stream, std::string& value) {
    char nextChar;
    stream >> nextChar;  // Skip '{'
    std::string key, val;
    std::ostringstream oss;

    while (getQuotedString(stream, key)) {
        if (!(stream >> std::ws && stream.get() == ':')) {
            std::cerr << "Invalid format: Key-value pair should be separated by ':'." << std::endl;
            return false;
        }

        if (!parseValue(stream, val)) {
            std::cerr << "Invalid format: Value should be a valid JSON type." << std::endl;
            return false;
        }

        oss << "\"" << key << "\": " << val;

        char nextChar;
        if (stream >> nextChar && nextChar == ',') {
            oss << ",";
        }
    }

    // Store the nested JSON object as a string for the current key
    value = "{" + oss.str() + "}";
    return true;
}


// Helper function to parse numbers, booleans, or other types (just numbers for now)
bool JSONReader::parseNumberOrBoolean(std::istringstream& stream, std::string& value) {
    std::ostringstream oss;
    char nextChar;
    while (stream.get(nextChar) && (isdigit(nextChar) || nextChar == '.' || nextChar == '-' || nextChar == '+')) {
        oss.put(nextChar);
    }

    if (oss.str().empty()) {
        return false;
    }

    value = oss.str();
    return true;
}

// Function to extract quoted string (key or value)
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

// Convert the internal data to a JSON string
std::string JSONReader::toJsonString() const {
    std::ostringstream oss;
    oss << "{";
    bool first = true;

    for (const auto& pair : jsonData) {
        if (!first) {
            oss << ",";
        }
        oss << "\"" << pair.first << "\":\"" << pair.second << "\"";
        first = false;
    }
    oss << "}";

    return oss.str();
}

// Add key-value pairs to the internal data
void JSONReader::add(const std::string& key, const std::string& value) {
    jsonData[key] = value;
}

