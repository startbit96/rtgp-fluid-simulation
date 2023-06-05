#include "helper.h"

std::string to_string_with_separator(unsigned int value, char thousand_sep)
{
    std::string result = std::to_string(value);
    int string_length = result.length();
    int separator_position = 3;

    while (string_length > separator_position) {
        result.insert(string_length - separator_position, 1, thousand_sep);
        separator_position += 4;
        string_length += 1;
    }
    return result;
}