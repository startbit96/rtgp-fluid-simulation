#pragma once

#include <string>
#include <vector>

#include "input.h"

class Input_Context {
    private:
        std::vector<Input> registered_inputs;
        std::string description;

    public:
        Input_Context (std::string description = "");

        void add_input (int key, void (*reaction)(), std::string description);
        void clear_inputs ();
        // Gets the list of pressed keys and passes it down to the 
        // registered input.
        void check_and_execute (std::vector<int> pressed_keys);
        void print_information ();
        // For the visualization handler to be able to print the key bindings in a table we will need to 
        // create a key - reaction string pair.
        void create_key_binding_list (std::vector<std::string> &key_list, std::vector<std::string> &reaction_description_list);
};