#pragma once

#include <string>

// A function that converts a number to a string and adds thousand separator.
// This is going to be used to print the number of particles more readable.
std::string to_string_with_separator(unsigned int value, char thousand_sep = ',');