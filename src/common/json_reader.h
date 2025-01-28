#ifndef JSON_READER_H
#define JSON_READER_H

#include <string>
#include <unordered_map>
#include <sstream>
#include <memory>  // For std::shared_ptr

class JSONReader {
public:
    bool parse(const std::string& input);
    std::string get(const std::string& key) const;
    void display() const;

private:
    std::unordered_map<std::string, std::shared_ptr<JSONReader>> jsonData;  // Nested JSON support

    void removeWhitespace(std::string& str);
    bool getQuotedString(std::istringstream& stream, std::string& str);
    bool parseValue(std::istringstream& stream, std::string& value);
    bool parseObject(std::istringstream& stream);
};

#endif  // JSON_READER_H
