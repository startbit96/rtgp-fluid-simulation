#pragma once

#include <string>

// A function that converts a number to a string and adds thousand separator.
// This is going to be used to print the number of particles more readable.
std::string to_string_with_separator(unsigned int value, char thousand_sep = ',')
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

std::string to_string (glm::vec3 vector)
{
    return "x: " + std::to_string(vector.x) + 
        ", y: "  + std::to_string(vector.y) + 
        ", z: "  + std::to_string(vector.z);
}

// Simply comparing floats like a == b can result in wrong results.
// Therefore create a function that compares floats depending on an allowed difference.
bool floats_are_same (float a, float b, float epsilon)
{
    return fabs(a - b) < epsilon;
}