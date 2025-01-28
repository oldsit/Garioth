#ifndef JSON_READER_H
#define JSON_READER_H

#include <string>
#include <unordered_map>
#include <sstream>
#include <memory>  // For std::shared_ptr

class JSONReader {
public:
    bool parse(const std::string& input);  // Parse JSON string into internal map
    std::string get(const std::string& key) const;  // Get value by key
    void display() const;  // For debugging, display parsed data as key-value pairs

    // Add a key-value pair to the internal map
    void add(const std::string& key, const std::string& value);

    // Convert the internal data into a JSON string
    std::string toJsonString() const;

    // Getter for the internal data (jsonData)
    std::unordered_map<std::string, std::string> getData() const;

private:
    std::unordered_map<std::string, std::string> jsonData;  // Simple key-value map for values

    void removeWhitespace(std::string& str);  // Helper to remove whitespaces from input
    bool getQuotedString(std::istringstream& stream, std::string& str);  // Extract quoted string (key or value)
    bool parseValue(std::istringstream& stream, std::string& value);  // Parse JSON values (string, number, object)
    bool parseObject(std::istringstream& stream, std::string& value);  // Parse nested JSON objects
    bool parseNumberOrBoolean(std::istringstream& stream, std::string& value);
};

#endif  // JSON_READER_H
