#include <iostream>
#include <algorithm>

#include "input_context.h"


Input_Context::Input_Context (std::string description)
{
    this->description = description;
}

void Input_Context::add_input (int key, void (*reaction)(), std::string description)
{
    this->registered_inputs.push_back(Input(key, reaction, description));
}

void Input_Context::clear_inputs () 
{
    this->registered_inputs.clear();
}

void Input_Context::check_and_execute (std::vector<int> pressed_keys)
{
    for (Input& input: this->registered_inputs) {
        for (int& pressed_key: pressed_keys) {
            if (input.check_and_execute(pressed_key) == true) {
                break;
            }
        }
    }
}

void Input_Context::print_information ()
{
    std::cout << this->description << std::endl;
    for (Input& input: this->registered_inputs) {
        input.print_information();
    }
}